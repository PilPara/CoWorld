/**
 * @file animator.hpp
 * @brief Defines the Animator class for skeletal animation playback
 */

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "animation.hpp"
#include "bone.hpp"
#include <glm/glm.hpp>
#include <map>
#include <vector>

/**
 * @enum AnimationLayer
 * @brief Animation layers for blending different body part animations
 */
enum class AnimationLayer {
  LOCOMOTION, ///< Base layer for walk, idle, run animations
  HEAD,       ///< Head look and expression animations
  TAIL        ///< Tail movement animations
};

/**
 * @class Animator
 * @brief Controls skeletal animation playback and blending
 *
 * Manages animation state, calculates bone transformations,
 * and supports layered animation blending for different body parts.
 * Provides final bone matrices for GPU skinning.
 */
class Animator {
public:
  /**
   * @brief Constructor
   * @param animation Initial animation to play (can be nullptr)
   */
  Animator(Animation *animation);

  /**
   * @brief Update animation state
   * @param dt Delta time in seconds
   */
  void UpdateAnimation(float dt);

  /**
   * @brief Play animation on main layer
   * @param animation Animation to play
   */
  void PlayAnimation(Animation *animation);

  /**
   * @brief Play animation on specific layer
   * @param animation Animation to play
   * @param layer Target animation layer
   */
  void PlayAnimationOnLayer(Animation *animation, AnimationLayer layer);

  /**
   * @brief Play animation on layer with loop control
   * @param animation Animation to play
   * @param layer Target animation layer
   * @param looping Whether to loop the animation
   */
  void PlayAnimationOnLayer(Animation *animation, AnimationLayer layer,
                            bool looping);

  /**
   * @brief Stop animation on specific layer
   * @param layer Layer to stop
   */
  void StopLayer(AnimationLayer layer);

  /**
   * @brief Get final bone transformation matrices
   * @return Vector of matrices for GPU skinning
   */
  std::vector<glm::mat4> GetFinalBoneMatrices();

  /**
   * @brief Override bone transformation
   * @param boneID ID of bone to override
   * @param overrideTransform Transformation to apply
   */
  void SetBoneOverride(int boneID, const glm::mat4 &overrideTransform);

  /**
   * @brief Clear bone override
   * @param boneID ID of bone to reset
   */
  void ClearBoneOverride(int boneID);

  /**
   * @brief Get current main animation
   * @return Currently playing animation or nullptr
   */
  Animation *GetCurrentAnimation() const { return this->currentAnimation; }

private:
  /**
   * @struct LayerInfo
   * @brief Animation state for a specific layer
   */
  struct LayerInfo {
    Animation *animation = nullptr; ///< Animation playing on this layer
    float currentTime = 0.0f;       ///< Current playback time
    bool active = false;            ///< Whether layer is active
    bool looping = true;            ///< Whether to loop animation
  };

  /**
   * @brief Calculate bone transforms for main animation
   * @param node Current node in hierarchy
   * @param parentTransform Parent's transformation matrix
   */
  void CalculateBoneTransform(const AssimpNodeData *node,
                              glm::mat4 parentTransform);

  /**
   * @brief Calculate transforms for specific layer
   * @param node Current node in hierarchy
   * @param parentTransform Parent's transformation matrix
   * @param layer Target animation layer
   * @param layerMatrices Output matrix array
   */
  void CalculateLayerBoneTransform(const AssimpNodeData *node,
                                   glm::mat4 parentTransform,
                                   AnimationLayer layer,
                                   std::vector<glm::mat4> &layerMatrices);

  std::vector<glm::mat4> finalBoneMatrices; ///< Final matrices for rendering
  std::map<int, glm::mat4> boneOverrides;   ///< Manual bone overrides

  std::map<AnimationLayer, LayerInfo> layers; ///< Layer animation states

  Animation *currentAnimation; ///< Main animation
  float currentTime;           ///< Main animation time
  float deltaTime;             ///< Frame delta time

  /**
   * @brief Check if bone belongs to head
   * @param boneName Name of bone to check
   * @return True if head bone
   */
  bool IsHeadBone(const std::string &boneName) const;

  /**
   * @brief Check if bone belongs to tail
   * @param boneName Name of bone to check
   * @return True if tail bone
   */
  bool IsTailBone(const std::string &boneName) const;

  /**
   * @brief Determine which layer a bone belongs to
   * @param boneName Name of bone to classify
   * @return Appropriate animation layer
   */
  AnimationLayer GetBoneLayer(const std::string &boneName) const;
};

#endif
