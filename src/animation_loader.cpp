/**
 * @file animation_loader.cpp
 * @brief Implementation of the AnimationLoader class
 */

#include "animation_loader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <algorithm>

/**
 * @brief Constructor - initializes loader with model file
 *
 * Scans the model file to build a catalog of available animations
 *
 * @param modelPath Path to the model file containing animations
 * @param model Target animated model for bone mapping
 */
AnimationLoader::AnimationLoader(const std::string &modelPath,
                                 AnimatedModel *model)
    : modelPath(modelPath), model(model) {
  LoadAnimationList();
}

/**
 * @brief Build catalog of available animations
 *
 * Uses Assimp to scan the model file and extract metadata
 * for all animation sequences. Stores name, index, duration,
 * and timing information for each animation.
 */
void AnimationLoader::LoadAnimationList() {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);

  // Check if file has animations
  if (!scene || !scene->HasAnimations()) {
    std::cerr << "[ERROR::ANIMATION_LOADER] No animations found in "
              << modelPath << std::endl;
    return;
  }

  // Clear and reserve space for animation list
  animations.clear();
  animations.reserve(scene->mNumAnimations);

  // Extract metadata for each animation
  for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
    const aiAnimation *anim = scene->mAnimations[i];

    AnimationInfo info;
    info.name = anim->mName.C_Str();
    info.index = i;
    info.duration = static_cast<float>(anim->mDuration);
    info.ticksPerSecond = anim->mTicksPerSecond != 0
                              ? static_cast<float>(anim->mTicksPerSecond)
                              : Config::Animation::DEFAULT_TICKS_PER_SECOND;

    animations.push_back(info);
  }
}

/**
 * @brief Load a specific animation by name
 *
 * Creates and returns an Animation object for the requested sequence.
 * Handles errors gracefully if animation not found or loading fails.
 *
 * @param name Name of the animation to load
 * @return Unique pointer to loaded animation, nullptr on failure
 */
std::unique_ptr<Animation>
AnimationLoader::LoadAnimation(const std::string &name) {
  // Find animation index
  int index = FindAnimationIndex(name);
  if (index == -1) {
    std::cerr << "[ANIMATION_LOADER] Animation not found: " << name
              << std::endl;
    return nullptr;
  }

  // Attempt to load animation
  try {
    return std::make_unique<Animation>(modelPath, model, index);
  } catch (const std::exception &e) {
    std::cerr << "[ANIMATION_LOADER] Failed to load animation '" << name
              << "': " << e.what() << std::endl;
    return nullptr;
  }
}

/**
 * @brief Get catalog of available animations
 * @return Reference to vector of animation metadata
 */
const std::vector<AnimationInfo> &
AnimationLoader::GetAvailableAnimations() const {
  return animations;
}

/**
 * @brief Find animation index by name
 *
 * Searches the animation catalog for a matching name
 *
 * @param name Animation name to search for
 * @return Index of the animation if found, -1 otherwise
 */
int AnimationLoader::FindAnimationIndex(const std::string &name) const {
  auto it = std::find_if(
      animations.begin(), animations.end(),
      [&name](const AnimationInfo &info) { return info.name == name; });

  return it != animations.end() ? it->index : -1;
}
