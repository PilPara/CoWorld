/**
 * @file bone.hpp
 * @brief Declaration of the Bone class for skeletal animation.
 *
 * Handles keyframe interpolation for position, rotation, and scaling
 * during skeletal animation playback.
 */

#ifndef BONE_H
#define BONE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>
#include <string>
#include <vector>
#include "assimpGLMHelpers.hpp"

/**
 * @brief Keyframe position data
 */
struct KeyPosition {
  glm::vec3 position;
  float timeStamp;
};

/**
 * @brief Keyframe rotation data
 */
struct KeyRotation {
  glm::quat orientation;
  float timeStamp;
};

/**
 * @brief Keyframe scaling data
 */
struct KeyScale {
  glm::vec3 scale;
  float timeStamp;
};

/**
 * @brief Represents a single bone in a skeletal animation
 */
class Bone {
public:
  Bone(const std::string &name, int ID, const aiNodeAnim *channel);

  /**
   * @brief Update local transform for current animation time
   * @param animationTime Current time in ticks
   */
  void Update(float animationTime);

  /**
   * @brief Get current local transformation matrix
   */
  glm::mat4 GetLocalTransform();

  /// @return Bone name
  std::string GetBoneName() const;

  /// @return Bone ID
  int GetBoneID();

private:
  std::string name;
  int ID;
  glm::mat4 localTransform;

  std::vector<KeyPosition> positions;
  std::vector<KeyRotation> rotations;
  std::vector<KeyScale> scales;
  int numPositions;
  int numRotations;
  int numScalings;

  int GetPositionIndex(float animationTime);
  int GetRotationIndex(float animationTime);
  int GetScaleIndex(float animationTime);

  float GetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                       float animationTime);

  glm::mat4 InterpolatePosition(float animationTime);
  glm::mat4 InterpolateRotation(float animationTime);
  glm::mat4 InterpolateScaling(float animationTime);
};

#endif
