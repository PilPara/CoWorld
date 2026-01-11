/**
 * @file animation.cpp
 * @brief Implementation of the Animation class for skeletal animation sequences
 */

#include "animation.hpp"
#include "animated_model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

/**
 * @brief Constructor - loads animation sequence from file
 *
 * Loads the specified animation from a model file using Assimp.
 * Extracts bone hierarchy, keyframes, and timing information.
 *
 * @param animationPath Path to the model file containing animations
 * @param model Target animated model for bone mapping
 * @param animationIndex Index of the animation to load (0-based)
 */
Animation::Animation(const std::string &animationPath, AnimatedModel *model,
                     int animationIndex) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      animationPath,
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  // Validate scene and animation index
  if (!scene || !scene->HasAnimations() ||
      animationIndex >= scene->mNumAnimations) {
    std::cerr << "[ERROR::ANIMATION] Failed to load " << animationPath
              << std::endl;
    return;
  }

  const aiAnimation *animation = scene->mAnimations[animationIndex];

  // Store animation name
  this->name = animation->mName.C_Str();

  // Extract timing information
  this->duration = static_cast<float>(animation->mDuration);
  this->ticksPerSecond =
      animation->mTicksPerSecond != 0
          ? static_cast<int>(animation->mTicksPerSecond)
          : static_cast<int>(Config::Animation::DEFAULT_TICKS_PER_SECOND);

  this->boneCount = 0;

  // Build hierarchy tree
  ReadHierarchyData(this->rootNode, scene->mRootNode);

  // Process animation channels and bones
  ReadMissingBones(animation, model);
}

/**
 * @brief Recursively build animation hierarchy
 *
 * Converts Assimp's node hierarchy into internal format,
 * preserving transformations and parent-child relationships.
 *
 * @param dest Destination node to populate
 * @param src Source Assimp node with hierarchy data
 */
void Animation::ReadHierarchyData(AssimpNodeData &dest, const aiNode *src) {
  // Copy node properties
  dest.name = src->mName.C_Str();
  dest.transformation =
      AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
  dest.childrenCount = src->mNumChildren;

  // Recursively process children
  for (unsigned int i = 0; i < src->mNumChildren; i++) {
    AssimpNodeData childData;
    ReadHierarchyData(childData, src->mChildren[i]);
    dest.children.push_back(childData);
  }
}

/**
 * @brief Process bones not already in the model
 *
 * Adds bone information for any bones referenced in the animation
 * but not present in the model's bone map. This ensures all animated
 * bones are properly tracked.
 *
 * @param animation Assimp animation containing bone channels
 * @param model Target model to update with missing bones
 */
void Animation::ReadMissingBones(const aiAnimation *animation,
                                 AnimatedModel *model) {
  int size = animation->mNumChannels;

  // Get references to model's bone data
  auto &boneInfoMap = model->GetBoneInfoMap();
  int &boneCount = model->GetBoneCount();

  // Process each animation channel
  for (int i = 0; i < size; i++) {
    auto channel = animation->mChannels[i];
    std::string boneName = channel->mNodeName.data;

    // Add bone if not already in model
    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
      boneInfoMap[boneName].id = boneCount;
      boneInfoMap[boneName].offset =
          glm::mat4(1.0f); // Identity matrix for missing bones
      boneCount++;
    }

    // Create bone object with keyframe data
    bones.push_back(Bone(channel->mNodeName.data,
                         boneInfoMap[channel->mNodeName.data].id, channel));
  }

  // Store final bone mapping
  this->boneInfoMap = boneInfoMap;
  this->boneCount = boneCount;
}

/**
 * @brief Find a bone by name
 *
 * Searches the bone collection for a bone with the specified name.
 *
 * @param name Name of the bone to find
 * @return Pointer to the bone if found, nullptr otherwise
 */
Bone *Animation::FindBone(const std::string &name) {
  auto iter = std::find_if(bones.begin(), bones.end(), [&](const Bone &bone) {
    return bone.GetBoneName() == name;
  });

  if (iter == bones.end())
    return nullptr;
  else
    return &(*iter);
}
