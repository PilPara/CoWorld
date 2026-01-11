/**
 * @file animation_loader.hpp
 * @brief Defines the AnimationLoader class for managing multiple animations
 */

#ifndef ANIMATION_LOADER_HPP
#define ANIMATION_LOADER_HPP

#include "animation.hpp"
#include "animated_model.hpp"
#include <string>
#include <vector>
#include <memory>

/**
 * @struct AnimationInfo
 * @brief Metadata for an available animation sequence
 */
struct AnimationInfo {
  std::string name;     ///< Animation name/identifier
  int index;            ///< Index in the model file
  float duration;       ///< Total duration in ticks
  float ticksPerSecond; ///< Playback speed
};

/**
 * @class AnimationLoader
 * @brief Manages loading and access to multiple animation sequences
 *
 * Provides a centralized interface for discovering and loading animations
 * from a model file. Maintains a catalog of available animations and
 * handles on-demand loading of specific sequences.
 */
class AnimationLoader {
public:
  /**
   * @brief Constructor - analyzes model for available animations
   * @param modelPath Path to the model file
   * @param model Target animated model for bone mapping
   */
  AnimationLoader(const std::string &modelPath, AnimatedModel *model);

  /**
   * @brief Load a specific animation by name
   * @param name Name of the animation to load
   * @return Unique pointer to loaded animation, nullptr if not found
   */
  std::unique_ptr<Animation> LoadAnimation(const std::string &name);

  /**
   * @brief Get list of all available animations
   * @return Vector of animation metadata
   */
  const std::vector<AnimationInfo> &GetAvailableAnimations() const;

  /**
   * @brief Find animation index by name
   * @param name Animation name to search for
   * @return Animation index if found, -1 otherwise
   */
  int FindAnimationIndex(const std::string &name) const;

private:
  std::string modelPath;                 ///< Path to the model file
  AnimatedModel *model;                  ///< Target model for animations
  std::vector<AnimationInfo> animations; ///< Catalog of available animations

  /**
   * @brief Scan model file for available animations
   */
  void LoadAnimationList();
};

#endif
