/**
 * @file scene.hpp
 * @brief Declaration of the Scene class and animated object structures.
 * 
 * This file defines the main Scene class that manages the entire 3D scene,
 * including models, cameras, lighting, animations, and user input handling.
 * It serves as the central coordinator for all scene elements.
 */

#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "model.hpp"
#include "animated_model.hpp"
#include "animation.hpp"
#include "animator.hpp"
#include "animation_loader.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "skybox.hpp"
#include "input_manager.hpp"

/**
 * @brief Container for an animated object and its associated animations.
 * 
 * Holds an animated model along with its various animation states
 * (idle, walk, run) and the animator that controls playback.
 */
struct AnimatedObject {
  std::unique_ptr<AnimatedModel> model;    ///< The animated 3D model
  std::unique_ptr<Animation> idleAnimation; ///< Animation for idle state
  std::unique_ptr<Animation> walkAnimation; ///< Animation for walking state  
  std::unique_ptr<Animation> runAnimation;  ///< Animation for running state
  std::unique_ptr<Animator> animator;       ///< Controls animation playback
};

/**
 * @brief Main scene class that manages the entire 3D environment.
 * 
 * The Scene class serves as the central coordinator for all scene elements including
 * 3D models, cameras, lighting, animations, and user input. It handles the complete
 * scene lifecycle from initialization through rendering and provides access to all
 * major scene components for external systems like ImGui interfaces.
 */
class Scene {
public:
  /**
   * @brief Default constructor that initializes default lighting.
   */
  Scene();

  /**
   * @brief Destructor that cleans up scene resources.
   */
  ~Scene();

  // =========================================================================
  // Scene Lifecycle
  // =========================================================================

  /**
   * @brief Initialize the complete scene with all its components.
   * @param windowWidth Width of the rendering window
   * @param windowHeight Height of the rendering window
   * 
   * Sets up shaders, models, cameras, animations, lighting, and skybox.
   * Must be called before any rendering or update operations.
   */
  void Initialize(int windowWidth, int windowHeight);

  /**
   * @brief Process user input for the current frame.
   * @param inputManager Input manager containing current input state
   * @param deltaTime Time elapsed since last frame in seconds
   * 
   * Handles camera switching, cow movement, and animation controls.
   */
  void HandleInput(const InputManager &inputManager, float deltaTime);

  /**
   * @brief Update scene state for the current frame.
   * @param deltaTime Time elapsed since last frame in seconds
   * 
   * Updates animations, camera positions, and other time-dependent elements.
   */
  void Update(float deltaTime);

  /**
   * @brief Render the complete scene.
   * 
   * Renders skybox, static objects, and animated objects in the correct order
   * using appropriate shaders and lighting setup.
   */
  void Render();

  // =========================================================================
  // Camera Management
  // =========================================================================

  /**
   * @brief Get pointer to the currently active camera.
   * @return Pointer to active camera, or nullptr if none exists
   */
  Camera *GetActiveCamera();

  /**
   * @brief Switch to the next camera in the sequence.
   * 
   * Cycles through available cameras in order, wrapping to first camera
   * after the last one.
   */
  void NextCamera();

  /**
   * @brief Set the active camera by index.
   * @param index Index of the camera to activate
   * 
   * Does nothing if index is out of range.
   */
  void SetActiveCamera(size_t index);

  /**
   * @brief Get camera by index.
   * @param index Index of the camera to retrieve
   * @return Pointer to camera, or nullptr if index is out of range
   */
  Camera *GetCamera(size_t index);

  /**
   * @brief Get the total number of cameras in the scene.
   * @return Number of available cameras
   */
  size_t GetCameraCount() const;

  // =========================================================================
  // Game Object Access
  // =========================================================================

  /**
   * @brief Get the first animated model in the scene.
   * @return Pointer to first animated model, or nullptr if none exists
   */
  AnimatedModel *GetFirstAnimatedModel();

  /**
   * @brief Get the animator for the first animated object.
   * @return Pointer to first animator, or nullptr if none exists
   */
  Animator *GetFirstAnimator();

  /**
   * @brief Get the idle animation for an animated object.
   * @param index Index of the animated object (default: 0)
   * @return Pointer to idle animation, or nullptr if not found
   */
  Animation *GetIdleAnimation(size_t index = 0);

  /**
   * @brief Get the walk animation for an animated object.
   * @param index Index of the animated object (default: 0)
   * @return Pointer to walk animation, or nullptr if not found
   */
  Animation *GetWalkAnimation(size_t index = 0);

  /**
   * @brief Get the run animation for an animated object.
   * @param index Index of the animated object (default: 0)
   * @return Pointer to run animation, or nullptr if not found
   */
  Animation *GetRunAnimation(size_t index = 0);

  /**
   * @brief Get reference to the collection of static models.
   * @return Reference to vector of static model unique pointers
   * 
   * Provides direct access to static models for collision detection
   * and other systems that need to iterate through scene objects.
   */
  std::vector<std::unique_ptr<Model>> &GetStaticModels() {
    return staticModels;
  }

  // =========================================================================
  // Lighting Management
  // =========================================================================

  /**
   * @brief Set the scene's directional light properties.
   * @param light DirectionalLight structure with light parameters
   */
  void SetDirectionalLight(const DirectionalLight &light);

  /**
   * @brief Add a point light to the scene.
   * @param light PointLight structure with light parameters
   */
  void AddPointLight(const PointLight &light);

  /**
   * @brief Remove all point lights from the scene.
   */
  void ClearPointLights() { pointLights.clear(); }

  // =========================================================================
  // Skybox Access
  // =========================================================================

  /**
   * @brief Get pointer to the scene's skybox.
   * @return Pointer to skybox, or nullptr if not initialized
   */
  Skybox *GetSkybox() { return skybox.get(); }

  // =========================================================================
  // Public Data (for ImGui and external access)
  // =========================================================================

  DirectionalLight directionalLight;        ///< Scene's directional light
  std::vector<PointLight> pointLights;      ///< Collection of point lights
  size_t activeCameraIndex = 0;             ///< Index of currently active camera

private:
  // =========================================================================
  // Initialization Methods
  // =========================================================================

  /**
   * @brief Initialize shader programs for static and animated objects.
   */
  void InitializeShaders();

  /**
   * @brief Load and setup all 3D models in the scene.
   * 
   * Creates ground plane, animated cow, farmhouse, tractor, shed,
   * and milk containers with their appropriate transformations.
   */
  void SetupModels();

  /**
   * @brief Setup cameras with proper perspective projection.
   * @param windowWidth Width of the rendering window
   * @param windowHeight Height of the rendering window
   * 
   * Creates free-fly, follow, and POV cameras with appropriate positioning.
   */
  void SetupCameras(int windowWidth, int windowHeight);

  /**
   * @brief Initialize default lighting setup.
   * 
   * Sets up directional light and any default point lights.
   */
  void SetupLighting();

  /**
   * @brief Load and setup animations for animated objects.
   * 
   * Loads idle and walk animations for the cow and sets up animators.
   */
  void SetupAnimations();

  /**
   * @brief Initialize the skybox with cubemap textures.
   */
  void SetupSkybox();

  // =========================================================================
  // Input Handling Methods
  // =========================================================================

  /**
   * @brief Handle cow movement based on user input.
   * @param inputManager Input manager containing current input state
   * @param deltaTime Time elapsed since last frame in seconds
   * 
   * Processes movement keys and updates cow position with collision detection.
   * Automatically switches between idle and walk animations.
   */
  void HandleCowMovement(const InputManager &inputManager, float deltaTime);

  /**
   * @brief Handle animation-specific input (head/tail movements).
   * @param inputManager Input manager containing current input state
   * 
   * Processes keys for playing head and tail animations on separate layers.
   */
  void HandleAnimationInput(const InputManager &inputManager);

  /**
   * @brief Handle camera-related input (switching, etc.).
   * @param inputManager Input manager containing current input state
   */
  void HandleCameraInput(const InputManager &inputManager);

  // =========================================================================
  // Update Methods
  // =========================================================================

  /**
   * @brief Update camera positions based on cow movement.
   * 
   * Updates follow and POV cameras to maintain proper positioning
   * relative to the animated cow.
   */
  void UpdateCameraPositions();

  /**
   * @brief Set the movement state of an animated model.
   * @param index Index of the animated object
   * @param moving Whether the model is currently moving
   * 
   * Switches between idle and walk animations based on movement state.
   */
  void SetAnimatedModelMoving(size_t index, bool moving);

  // =========================================================================
  // Rendering Methods
  // =========================================================================

  /**
   * @brief Render all static objects in the scene.
   * 
   * Sets up static shader, lighting, and renders all non-animated models.
   */
  void RenderStaticObjects();

  /**
   * @brief Render all animated objects in the scene.
   * 
   * Sets up animated shader, bone matrices, and renders animated models.
   */
  void RenderAnimatedObjects();

  /**
   * @brief Configure lighting uniforms in a shader.
   * @param shader Shader to configure with lighting data
   * 
   * Sets directional light, point lights, and light counts in the shader.
   */
  void SetupLighting(Shader &shader);

  // =========================================================================
  // Scene Data
  // =========================================================================

  std::vector<std::unique_ptr<Camera>> cameras;          ///< All scene cameras
  std::vector<std::unique_ptr<Model>> staticModels;      ///< Static 3D models
  std::vector<AnimatedObject> animatedObjects;           ///< Animated objects with animations

  // =========================================================================
  // Rendering Resources
  // =========================================================================

  std::unique_ptr<Shader> staticShader;                  ///< Shader for static objects
  std::unique_ptr<Shader> animatedShader;                ///< Shader for animated objects
  std::unique_ptr<Skybox> skybox;                        ///< Scene skybox

  // =========================================================================
  // Animation Resources
  // =========================================================================

  std::unique_ptr<AnimationLoader> animationLoader;      ///< Loads animations from model files
};

#endif
