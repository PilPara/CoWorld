// #include "bone.hpp"
// #include <cassert>
//
// Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel)
//     : name(name), ID(ID), localTransform(1.0f) {
//
//   this->numPositions = channel->mNumPositionKeys;
//   for (int positionIndex = 0; positionIndex < this->numPositions;
//        ++positionIndex) {
//     aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
//     float timeStamp = channel->mPositionKeys[positionIndex].mTime;
//     KeyPosition data;
//     data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
//     data.timeStamp = timeStamp;
//     this->positions.push_back(data);
//   }
//
//   this->numRotations = channel->mNumRotationKeys;
//   for (int rotationIndex = 0; rotationIndex < this->numRotations;
//        ++rotationIndex) {
//     aiQuaternion aiOrientation =
//     channel->mRotationKeys[rotationIndex].mValue; float timeStamp =
//     channel->mRotationKeys[rotationIndex].mTime; KeyRotation data;
//     data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
//     data.timeStamp = timeStamp;
//     this->rotations.push_back(data);
//   }
//
//   this->numScalings = channel->mNumScalingKeys;
//   for (int keyIndex = 0; keyIndex < this->numScalings; ++keyIndex) {
//     aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
//     float timeStamp = channel->mScalingKeys[keyIndex].mTime;
//     KeyScale data;
//     data.scale = AssimpGLMHelpers::GetGLMVec(scale);
//     data.timeStamp = timeStamp;
//     this->scales.push_back(data);
//   }
// }
//
// void Bone::Update(float animationTime) {
//   glm::mat4 translation = InterpolatePosition(animationTime);
//   glm::mat4 rotation = InterpolateRotation(animationTime);
//   glm::mat4 scale = InterpolateScaling(animationTime);
//   this->localTransform = translation * rotation * scale;
// }
//
// glm::mat4 Bone::GetLocalTransform() { return this->localTransform; }
//
// std::string Bone::GetBoneName() const { return this->name; }
//
// int Bone::GetBoneID() { return this->ID; }
//
// int Bone::GetPositionIndex(float animationTime) {
//   for (int index = 0; index < this->numPositions - 1; ++index) {
//     if (animationTime < this->positions[index + 1].timeStamp)
//       return index;
//   }
//   return this->numPositions - 1;
// }
//
// int Bone::GetRotationIndex(float animationTime) {
//   for (int index = 0; index < this->numRotations - 1; ++index) {
//     if (animationTime < this->rotations[index + 1].timeStamp)
//       return index;
//   }
//   return this->numRotations - 1;
// }
//
// int Bone::GetScaleIndex(float animationTime) {
//   for (int index = 0; index < this->numScalings - 1; ++index) {
//     if (animationTime < this->scales[index + 1].timeStamp)
//       return index;
//   }
//   return this->numScalings - 1;
// }
//
// float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp,
//                            float animationTime) {
//   float midWayLength = animationTime - lastTimeStamp;
//   float framesDiff = nextTimeStamp - lastTimeStamp;
//   return midWayLength / framesDiff;
// }
//
// glm::mat4 Bone::InterpolatePosition(float animationTime) {
//   if (1 == this->numPositions)
//     return glm::translate(glm::mat4(1.0f), this->positions[0].position);
//
//   int p0Index = GetPositionIndex(animationTime);
//   int p1Index = p0Index + 1;
//   float scaleFactor =
//       GetScaleFactor(this->positions[p0Index].timeStamp,
//                      this->positions[p1Index].timeStamp, animationTime);
//   glm::vec3 finalPosition =
//       glm::mix(this->positions[p0Index].position,
//                this->positions[p1Index].position, scaleFactor);
//   return glm::translate(glm::mat4(1.0f), finalPosition);
// }
//
// glm::mat4 Bone::InterpolateRotation(float animationTime) {
//   if (1 == this->numRotations) {
//     auto rotation = glm::normalize(this->rotations[0].orientation);
//     return glm::mat4_cast(rotation);
//   }
//
//   int p0Index = GetRotationIndex(animationTime);
//   int p1Index = p0Index + 1;
//   float scaleFactor =
//       GetScaleFactor(this->rotations[p0Index].timeStamp,
//                      this->rotations[p1Index].timeStamp, animationTime);
//   glm::quat finalRotation =
//       glm::slerp(this->rotations[p0Index].orientation,
//                  this->rotations[p1Index].orientation, scaleFactor);
//   finalRotation = glm::normalize(finalRotation);
//   return glm::mat4_cast(finalRotation);
// }
//
// glm::mat4 Bone::InterpolateScaling(float animationTime) {
//   if (1 == this->numScalings)
//     return glm::scale(glm::mat4(1.0f), this->scales[0].scale);
//
//   int p0Index = GetScaleIndex(animationTime);
//   int p1Index = p0Index + 1;
//   float scaleFactor =
//       GetScaleFactor(this->scales[p0Index].timeStamp,
//                      this->scales[p1Index].timeStamp, animationTime);
//   glm::vec3 finalScale = glm::mix(this->scales[p0Index].scale,
//                                   this->scales[p1Index].scale, scaleFactor);
//   return glm::scale(glm::mat4(1.0f), finalScale);
// }

/**
 * @file bone.cpp
 * @brief Implementation of the Bone class for skeletal animation.
 */

#include "bone.hpp"
#include <cassert>

Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel)
    : name(name), ID(ID), localTransform(1.0f) {
  // --- Load position keyframes ---
  this->numPositions = channel->mNumPositionKeys;
  for (int i = 0; i < this->numPositions; ++i) {
    aiVector3D aiPos = channel->mPositionKeys[i].mValue;
    float t = channel->mPositionKeys[i].mTime;
    positions.push_back({AssimpGLMHelpers::GetGLMVec(aiPos), t});
  }

  // --- Load rotation keyframes ---
  this->numRotations = channel->mNumRotationKeys;
  for (int i = 0; i < this->numRotations; ++i) {
    aiQuaternion aiRot = channel->mRotationKeys[i].mValue;
    float t = channel->mRotationKeys[i].mTime;
    rotations.push_back({AssimpGLMHelpers::GetGLMQuat(aiRot), t});
  }

  // --- Load scaling keyframes ---
  this->numScalings = channel->mNumScalingKeys;
  for (int i = 0; i < this->numScalings; ++i) {
    aiVector3D aiScale = channel->mScalingKeys[i].mValue;
    float t = channel->mScalingKeys[i].mTime;
    scales.push_back({AssimpGLMHelpers::GetGLMVec(aiScale), t});
  }
}

void Bone::Update(float animationTime) {
  // Interpolate T, R, S independently and combine
  glm::mat4 T = InterpolatePosition(animationTime);
  glm::mat4 R = InterpolateRotation(animationTime);
  glm::mat4 S = InterpolateScaling(animationTime);

  this->localTransform = T * R * S;
}

glm::mat4 Bone::GetLocalTransform() { return localTransform; }
std::string Bone::GetBoneName() const { return name; }
int Bone::GetBoneID() { return ID; }

int Bone::GetPositionIndex(float animationTime) {
  // Find the index of the current and next keyframe
  for (int i = 0; i < numPositions - 1; ++i) {
    if (animationTime < positions[i + 1].timeStamp)
      return i;
  }
  return numPositions - 1;
}

int Bone::GetRotationIndex(float animationTime) {
  for (int i = 0; i < numRotations - 1; ++i) {
    if (animationTime < rotations[i + 1].timeStamp)
      return i;
  }
  return numRotations - 1;
}

int Bone::GetScaleIndex(float animationTime) {
  for (int i = 0; i < numScalings - 1; ++i) {
    if (animationTime < scales[i + 1].timeStamp)
      return i;
  }
  return numScalings - 1;
}

float Bone::GetScaleFactor(float last, float next, float t) {
  // Normalized factor between two keyframes [0,1]
  return (t - last) / (next - last);
}

glm::mat4 Bone::InterpolatePosition(float t) {
  if (numPositions == 1) {
    // Only one keyframe → static translation
    return glm::translate(glm::mat4(1.0f), positions[0].position);
  }

  // Linear interpolation between two position keyframes
  int i = GetPositionIndex(t), j = i + 1;
  float f = GetScaleFactor(positions[i].timeStamp, positions[j].timeStamp, t);
  glm::vec3 pos = glm::mix(positions[i].position, positions[j].position, f);
  return glm::translate(glm::mat4(1.0f), pos);
}

glm::mat4 Bone::InterpolateRotation(float t) {
  if (numRotations == 1) {
    // Only one keyframe → static orientation
    return glm::mat4_cast(glm::normalize(rotations[0].orientation));
  }

  // Spherical interpolation between rotation keyframes
  int i = GetRotationIndex(t), j = i + 1;
  float f = GetScaleFactor(rotations[i].timeStamp, rotations[j].timeStamp, t);
  glm::quat rot =
      glm::slerp(rotations[i].orientation, rotations[j].orientation, f);
  return glm::mat4_cast(glm::normalize(rot));
}

glm::mat4 Bone::InterpolateScaling(float t) {
  if (numScalings == 1) {
    // Only one keyframe → static scale
    return glm::scale(glm::mat4(1.0f), scales[0].scale);
  }

  // Linear interpolation between scale keyframes
  int i = GetScaleIndex(t), j = i + 1;
  float f = GetScaleFactor(scales[i].timeStamp, scales[j].timeStamp, t);
  glm::vec3 scale = glm::mix(scales[i].scale, scales[j].scale, f);
  return glm::scale(glm::mat4(1.0f), scale);
}
