/**
 * @file animator.cpp
 * @brief Implementation of the Animator class for skeletal animation playback
 */

#include "animator.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
 * @brief Constructor
 *
 * Initializes animator with optional starting animation.
 * Allocates bone matrix buffer based on animation or config defaults.
 *
 * @param animation Initial animation to play (can be nullptr)
 */
Animator::Animator(Animation *animation) {
  this->currentTime = 0.0f;
  this->deltaTime = 0.0f;
  this->currentAnimation = animation;

  int boneCount =
      (animation ? animation->GetBoneCount() : Config::Animation::MAX_BONES);
  this->finalBoneMatrices.resize(boneCount, glm::mat4(1.0f));
}

/**
 * @brief Update all animation states
 *
 * Advances animation timers, handles looping, processes layer animations,
 * and calculates final bone transformation matrices for the current frame.
 *
 * @param dt Delta time in seconds since last update
 */
void Animator::UpdateAnimation(float dt) {
  this->deltaTime = dt;

  // Update main animation time
  if (this->currentAnimation) {
    float oldTime = this->currentTime;
    this->currentTime += this->currentAnimation->GetTicksPerSecond() * dt;
    this->currentTime =
        fmod(this->currentTime, this->currentAnimation->GetDuration());

    // Frame counter for debugging
    if (abs(this->currentTime - oldTime) >
        Config::Animation::EPSILON_TIME_DELTA) {
      static int frameCount = 0;
      frameCount++;
    }
  }

  // Update layer animations
  for (auto &[layer, info] : this->layers) {
    if (info.active && info.animation) {
      info.currentTime += info.animation->GetTicksPerSecond() * dt;

      // Handle non-looping animations
      if (!info.looping && info.currentTime >= info.animation->GetDuration()) {
        info.active = false;
        info.animation = nullptr;
        info.currentTime = 0.0f;
      } else if (info.looping) {
        // Loop animation
        info.currentTime =
            fmod(info.currentTime, info.animation->GetDuration());
      }
    }
  }

  // Reset matrices to identity
  std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(),
            glm::mat4(1.0f));

  // Calculate main animation transforms
  if (this->currentAnimation) {
    CalculateBoneTransform(&this->currentAnimation->GetRootNode(),
                           glm::mat4(1.0f));
  }

  // Apply layer animations (override specific bones)
  for (auto &[layer, info] : this->layers) {
    if (info.active && info.animation) {
      std::vector<glm::mat4> layerMatrices(finalBoneMatrices.size(),
                                           glm::mat4(1.0f));
      CalculateLayerBoneTransform(&info.animation->GetRootNode(),
                                  glm::mat4(1.0f), layer, layerMatrices);

      // Apply layer transforms to appropriate bones
      auto boneInfoMap = info.animation->GetBoneIDMap();
      for (const auto &[boneName, boneInfo] : boneInfoMap) {
        if (GetBoneLayer(boneName) == layer && boneInfo.id >= 0 &&
            boneInfo.id < finalBoneMatrices.size()) {
          finalBoneMatrices[boneInfo.id] = layerMatrices[boneInfo.id];
        }
      }
    }
  }
}

/**
 * @brief Check if bone belongs to head
 *
 * Matches bone name against head bone filters from config
 *
 * @param boneName Name of bone to check
 * @return True if bone is part of head
 */
bool Animator::IsHeadBone(const std::string &boneName) const {
  for (auto *filter : Config::Animation::HEAD_BONE_FILTERS) {
    if (boneName.find(filter) != std::string::npos) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Check if bone belongs to tail
 *
 * Matches bone name against tail bone filters from config
 *
 * @param boneName Name of bone to check
 * @return True if bone is part of tail
 */
bool Animator::IsTailBone(const std::string &boneName) const {
  for (auto *filter : Config::Animation::TAIL_BONE_FILTERS) {
    if (boneName.find(filter) != std::string::npos) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Play animation on main layer
 *
 * Only resets time if switching to a different animation
 *
 * @param animation Animation to play
 */
void Animator::PlayAnimation(Animation *animation) {
  // Only reset time if switching to a different animation
  if (this->currentAnimation != animation) {
    this->currentAnimation = animation;
    this->currentTime = 0.0f; // Reset animation time

    if (animation) {
      int boneCount = animation->GetBoneCount();
      this->finalBoneMatrices.resize(boneCount, glm::mat4(1.0f));
    }
  }
}

/**
 * @brief Play animation on specific layer
 *
 * @param animation Animation to play
 * @param layer Target layer (HEAD, TAIL, LOCOMOTION)
 */
void Animator::PlayAnimationOnLayer(Animation *animation,
                                    AnimationLayer layer) {
  auto &info = this->layers[layer];
  info.animation = animation;
  info.currentTime = 0.0f;
  info.active = (animation != nullptr);
}

/**
 * @brief Stop animation on layer
 * @param layer Layer to stop
 */
void Animator::StopLayer(AnimationLayer layer) {
  PlayAnimationOnLayer(nullptr, layer);
}

/**
 * @brief Get final bone matrices for rendering
 * @return Vector of transformation matrices for GPU skinning
 */
std::vector<glm::mat4> Animator::GetFinalBoneMatrices() {
  return this->finalBoneMatrices;
}

/**
 * @brief Calculate bone transforms recursively
 *
 * Traverses bone hierarchy, applies animations and overrides,
 * and generates final transformation matrices.
 *
 * @param node Current node in hierarchy
 * @param parentTransform Accumulated parent transformation
 */
void Animator::CalculateBoneTransform(const AssimpNodeData *node,
                                      glm::mat4 parentTransform) {
  std::string nodeName = node->name;
  glm::mat4 nodeTransform = node->transformation;

  // Update with animation if bone exists
  Bone *bone = this->currentAnimation->FindBone(nodeName);
  if (bone) {
    bone->Update(this->currentTime);
    nodeTransform = bone->GetLocalTransform();
  }

  // Apply manual overrides
  auto boneInfoMap = this->currentAnimation->GetBoneIDMap();
  auto it = boneInfoMap.find(nodeName);
  if (it != boneInfoMap.end()) {
    int index = it->second.id;
    if (index >= 0 && index < this->finalBoneMatrices.size()) {
      auto overrideIt = this->boneOverrides.find(index);
      if (overrideIt != this->boneOverrides.end()) {
        nodeTransform = overrideIt->second * nodeTransform;
      }
    }
  }

  glm::mat4 globalTransformation = parentTransform * nodeTransform;

  // Set final bone matrix
  if (it != boneInfoMap.end()) {
    int index = it->second.id;
    if (index >= 0 && index < this->finalBoneMatrices.size()) {
      glm::mat4 offset = it->second.offset;
      this->finalBoneMatrices[index] = globalTransformation * offset;
    }
  }

  // Recurse to children
  for (int i = 0; i < node->childrenCount; i++) {
    CalculateBoneTransform(&node->children[i], globalTransformation);
  }
}

/**
 * @brief Calculate transforms for layer animation
 *
 * Similar to CalculateBoneTransform but only affects bones
 * belonging to the specified layer.
 *
 * @param node Current node in hierarchy
 * @param parentTransform Accumulated parent transformation
 * @param layer Target animation layer
 * @param layerMatrices Output matrix array for this layer
 */
void Animator::CalculateLayerBoneTransform(
    const AssimpNodeData *node, glm::mat4 parentTransform, AnimationLayer layer,
    std::vector<glm::mat4> &layerMatrices) {
  std::string nodeName = node->name;
  glm::mat4 nodeTransform = node->transformation;

  auto &info = this->layers[layer];

  // Update with layer animation
  Bone *bone = info.animation->FindBone(nodeName);
  if (bone) {
    bone->Update(info.currentTime);
    nodeTransform = bone->GetLocalTransform();
  }

  glm::mat4 globalTransformation = parentTransform * nodeTransform;

  // Set bone matrix only for bones belonging to this layer
  auto boneInfoMap = info.animation->GetBoneIDMap();
  auto it = boneInfoMap.find(nodeName);
  if (it != boneInfoMap.end() && GetBoneLayer(nodeName) == layer) {
    int index = it->second.id;
    if (index >= 0 && index < layerMatrices.size()) {
      glm::mat4 offset = it->second.offset;
      layerMatrices[index] = globalTransformation * offset;
    }
  }

  // Recurse to children
  for (int i = 0; i < node->childrenCount; i++) {
    CalculateLayerBoneTransform(&node->children[i], globalTransformation, layer,
                                layerMatrices);
  }
}

/**
 * @brief Set manual bone override
 * @param boneID ID of bone to override
 * @param overrideTransform Transformation to apply
 */
void Animator::SetBoneOverride(int boneID, const glm::mat4 &overrideTransform) {
  this->boneOverrides[boneID] = overrideTransform;
}

/**
 * @brief Clear bone override
 * @param boneID ID of bone to reset
 */
void Animator::ClearBoneOverride(int boneID) {
  this->boneOverrides.erase(boneID);
}

/**
 * @brief Determine animation layer for bone
 * @param boneName Name of bone
 * @return Appropriate layer (HEAD, TAIL, or LOCOMOTION)
 */
AnimationLayer Animator::GetBoneLayer(const std::string &boneName) const {
  if (IsHeadBone(boneName))
    return AnimationLayer::HEAD;
  if (IsTailBone(boneName))
    return AnimationLayer::TAIL;
  return AnimationLayer::LOCOMOTION;
}

/**
 * @brief Play animation on layer with loop control
 * @param animation Animation to play
 * @param layer Target layer
 * @param looping Whether to loop the animation
 */
void Animator::PlayAnimationOnLayer(Animation *animation, AnimationLayer layer,
                                    bool looping) {
  auto &info = this->layers[layer];
  info.animation = animation;
  info.currentTime = 0.0f;
  info.active = (animation != nullptr);
  info.looping = looping;
}
