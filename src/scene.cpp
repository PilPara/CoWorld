/**
 * @file scene.cpp
 * @brief Implementation of the Scene class and scene management functionality.
 * 
 * Contains the complete implementation of the Scene class, which serves as the
 * central coordinator for the 3D scene. Handles initialization, input processing,
 * updates, and rendering of all scene elements including models, cameras,
 * lighting, animations, and skybox.
 */

#include "scene.hpp"
#include "plane.hpp"
#include "animated_model.hpp"
#include "animation.hpp"
#include "animator.hpp"
#include "config.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

/**
 * @brief Default constructor that initializes default lighting.
 * 
 * Sets up the default directional light using values from the Config class.
 */
Scene::Scene() {
  this->directionalLight = {glm::normalize(Config::Lighting::DEFAULT_DIR_LIGHT),
                            Config::Lighting::DIR_AMBIENT,
                            Config::Lighting::DIR_DIFFUSE,
                            Config::Lighting::DIR_SPECULAR};
}

/**
 * @brief Destructor that cleans up scene resources.
 * 
 * Resources are automatically cleaned up by smart pointers,
 * but this provides an explicit cleanup point if needed.
 */
Scene::~Scene() {}

/**
 * @brief Initialize the complete scene with all its components.
 * @param windowWidth Width of the rendering window
 * @param windowHeight Height of the rendering window
 * 
 * Performs complete scene initialization by calling all setup methods
 * in the correct order. Must be called before any rendering operations.
 */
void Scene::Initialize(int windowWidth, int windowHeight) {
  InitializeShaders();
  SetupModels();
  SetupCameras(windowWidth, windowHeight);
  SetupAnimations();
  SetupLighting();
  SetupSkybox();
}

/**
 * @brief Initialize shader programs for static and animated objects.
 * 
 * Creates shader programs using vertex and fragment shader paths
 * defined in the Config class. Separate shaders are used for
 * static objects and animated objects with bone matrices.
 */
void Scene::InitializeShaders() {
  this->staticShader =
      std::make_unique<Shader>(Config::Scene::STATIC_VERTEX_SHADER,
                               Config::Scene::STATIC_FRAGMENT_SHADER);

  this->animatedShader =
      std::make_unique<Shader>(Config::Scene::ANIMATED_VERTEX_SHADER,
                               Config::Scene::ANIMATED_FRAGMENT_SHADER);
}

/**
 * @brief Load and setup all 3D models in the scene.
 * 
 * Creates and configures all scene objects including:
 * - Ground plane for the base surface
 * - Animated cow model with default positioning
 * - Static objects: farmhouse, tractor, shed
 * - Multiple milk containers arranged in a row
 * 
 * All models are loaded with appropriate transformations and scaling.
 */
void Scene::SetupModels() {
  // Ground plane
  auto plane = std::make_unique<Plane>(glm::vec3(0.0f), glm::vec3(1.0f));
  plane->Init();
  this->staticModels.push_back(std::move(plane));

  // Animated cow
  try {
    auto cow = std::make_unique<AnimatedModel>(Config::Cow::DEFAULT_POSITION,
                                               Config::Cow::DEFAULT_SIZE);
    cow->LoadModel(Config::Assets::COW_MODEL);
    this->animatedObjects.push_back(
        AnimatedObject{std::move(cow), nullptr, nullptr, nullptr, nullptr});
  } catch (const std::exception &e) {
    std::cout << Config::Scene::ERR_COW_LOAD << ": " << e.what() << std::endl;
  }

  // Farmhouse
  auto farmhouse =
      std::make_unique<Model>(Config::ModelTransforms::FARMHOUSE_POS,
                              Config::ModelTransforms::FARMHOUSE_SCALE, false);
  farmhouse->LoadModel(Config::Assets::FARMHOUSE_MODEL);
  farmhouse->Rotate(Config::ModelTransforms::FARMHOUSE_ROTATION.x,
                    glm::vec3(1.0f, 0.0f, 0.0f));
  this->staticModels.push_back(std::move(farmhouse));

  // Tractor
  auto tractor =
      std::make_unique<Model>(Config::ModelTransforms::TRACTOR_POS,
                              Config::ModelTransforms::TRACTOR_SCALE, false);
  tractor->LoadModel(Config::Assets::TRACTOR_MODEL);
  tractor->Rotate(Config::ModelTransforms::TRACTOR_ROTATION.x,
                  glm::vec3(1.0f, 0.0f, 0.0f));
  this->staticModels.push_back(std::move(tractor));

  // Shed
  auto shed =
      std::make_unique<Model>(Config::ModelTransforms::SHED_POS,
                              Config::ModelTransforms::SHED_SCALE, false);
  shed->LoadModel(Config::Assets::SHED_MODEL);
  shed->Rotate(Config::ModelTransforms::SHED_ROTATION.x,
               glm::vec3(1.0f, 0.0f, 0.0f));
  this->staticModels.push_back(std::move(shed));

  // Milk containers
  for (int i = 0; i < Config::ModelTransforms::MILK_CAN_COUNT; i++) {
    glm::vec3 pos = Config::ModelTransforms::MILK_CAN_BASE_POS +
                    (float)i * Config::ModelTransforms::MILK_CAN_OFFSET;
    auto milkContainer = std::make_unique<Model>(
        pos, Config::ModelTransforms::MILK_CAN_SCALE, false);
    milkContainer->LoadModel(Config::Assets::MILK_CAN_MODEL);
    milkContainer->Rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    this->staticModels.push_back(std::move(milkContainer));
  }
}

/**
 * @brief Setup cameras with proper perspective projection.
 * @param windowWidth Width of the rendering window
 * @param windowHeight Height of the rendering window
 * 
 * Creates three types of cameras:
 * 1. Free-fly camera for general scene observation
 * 2. Follow camera that tracks the cow from behind
 * 3. POV camera that shows the cow's first-person view
 * 
 * All cameras use the same perspective projection parameters.
 */
void Scene::SetupCameras(int windowWidth, int windowHeight) {
  float FOV = glm::radians(Config::Camera::DEFAULT_FOV);
  PersProjInfo persProjInfo = {FOV, (float)windowWidth, (float)windowHeight,
                               Config::Camera::NEAR_PLANE,
                               Config::Camera::FAR_PLANE};

  glm::vec3 upDir = Config::Camera::UP_DIR;
  AnimatedModel *cow = GetFirstAnimatedModel();
  glm::vec3 cowCenter =
      cow ? cow->pos + glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0, 1, 0);

  // Free fly camera
  auto freeCam = std::make_unique<Camera>(Config::Camera::FREE_POS,
                                          Config::Camera::FREE_TARGET, upDir,
                                          persProjInfo);
  this->cameras.push_back(std::move(freeCam));

  // Follow camera
  glm::vec3 followPos = cowCenter + Config::Camera::FOLLOW_OFFSET;
  auto followCam =
      std::make_unique<Camera>(followPos, cowCenter, upDir, persProjInfo);
  this->cameras.push_back(std::move(followCam));

  // POV camera
  glm::vec3 cowEyePos = cowCenter + Config::Camera::POV_EYE_OFFSET;
  glm::vec3 cowLookTarget = cowCenter + Config::Camera::POV_LOOK_OFFSET;
  auto cowPOVCam =
      std::make_unique<Camera>(cowEyePos, cowLookTarget, upDir, persProjInfo);
  this->cameras.push_back(std::move(cowPOVCam));
}

/**
 * @brief Load and setup animations for animated objects.
 * 
 * Initializes the animation loader and loads idle and walk animations
 * for the cow. Sets up the animator with the idle animation as default.
 * Handles fallback animations if primary animations are not found.
 */
void Scene::SetupAnimations() {
  if (animatedObjects.empty())
    return;

  AnimatedModel *cow = animatedObjects[0].model.get();
  if (!cow)
    return;

  this->animationLoader =
      std::make_unique<AnimationLoader>(Config::Assets::COW_MODEL, cow);

  auto idleAnim =
      animationLoader->LoadAnimation(Config::Scene::ANIM_IDLE_PRIMARY);
  auto walkAnim =
      animationLoader->LoadAnimation(Config::Scene::ANIM_WALK_PRIMARY);
  if (!idleAnim)
    idleAnim =
        animationLoader->LoadAnimation(Config::Scene::ANIM_IDLE_FALLBACK);
  if (!walkAnim)
    walkAnim =
        animationLoader->LoadAnimation(Config::Scene::ANIM_WALK_FALLBACK);

  Animation *animPtr = idleAnim.get();
  auto animator = std::make_unique<Animator>(animPtr);

  animatedObjects[0].idleAnimation = std::move(idleAnim);
  animatedObjects[0].walkAnimation = std::move(walkAnim);
  animatedObjects[0].animator = std::move(animator);
}

/**
 * @brief Initialize default lighting setup.
 * 
 * Creates a point light near the farmhouse with appropriate
 * attenuation parameters and color values from the Config class.
 */
void Scene::SetupLighting() {
  PointLight houseLight;
  houseLight.position = Config::Lighting::HOUSE_LIGHT_POS;
  houseLight.k0 = Config::Lighting::HOUSE_LIGHT_K0;
  houseLight.k1 = Config::Lighting::HOUSE_LIGHT_K1;
  houseLight.k2 = Config::Lighting::HOUSE_LIGHT_K2;
  houseLight.ambient = Config::Lighting::HOUSE_AMBIENT;
  houseLight.diffuse = Config::Lighting::HOUSE_DIFFUSE;
  houseLight.specular = Config::Lighting::HOUSE_SPECULAR;

  this->pointLights.push_back(houseLight);
}

/**
 * @brief Initialize the skybox with cubemap textures.
 * 
 * Creates the skybox using texture faces defined in the Config class.
 * The skybox provides the background environment for the scene.
 */
void Scene::SetupSkybox() {
  std::vector<std::string> faces(Config::Assets::SKYBOX_FACES.begin(),
                                 Config::Assets::SKYBOX_FACES.end());
  this->skybox = std::make_unique<Skybox>(faces);
}

/**
 * @brief Process user input for the current frame.
 * @param inputManager Input manager containing current input state
 * @param deltaTime Time elapsed since last frame in seconds
 * 
 * Delegates input handling to specialized methods for different
 * input categories: camera controls, cow movement, and animations.
 */
void Scene::HandleInput(const InputManager &inputManager, float deltaTime) {
  HandleCameraInput(inputManager);
  HandleCowMovement(inputManager, deltaTime);
  HandleAnimationInput(inputManager);
}

/**
 * @brief Handle camera-related input (switching, etc.).
 * @param inputManager Input manager containing current input state
 * 
 * Processes camera switching input and other camera-related controls.
 * Camera reset functionality is reserved for future implementation.
 */
void Scene::HandleCameraInput(const InputManager &inputManager) {
  if (inputManager.IsPressed(InputAction::NEXT_CAMERA)) {
    NextCamera();
  }
  if (inputManager.IsPressed(InputAction::RESET_CAMERA)) {
    // Reset camera orientation - could be implemented later
  }
}

/**
 * @brief Handle cow movement based on user input.
 * @param inputManager Input manager containing current input state
 * @param deltaTime Time elapsed since last frame in seconds
 * 
 * Processes movement input differently based on active camera:
 * - Free/Follow cameras: WASD controls for directional movement
 * - POV camera: Forward movement in camera direction
 * 
 * Includes collision detection, automatic animation switching between
 * idle and walk states, and camera position updates.
 */
void Scene::HandleCowMovement(const InputManager &inputManager,
                              float deltaTime) {
  AnimatedModel *cow = GetFirstAnimatedModel();
  Animator *animator = GetFirstAnimator();
  if (!cow || !animator)
    return;

  bool moving = false;
  float speed = Config::Cow::MOVEMENT_SPEED;

  if (activeCameraIndex == Config::Scene::FREE_CAMERA_INDEX ||
      activeCameraIndex == Config::Scene::FOLLOW_CAMERA_INDEX) {
    glm::vec3 movement(0.0f);

    if (inputManager.IsHeld(InputAction::COW_FORWARD))
      movement += glm::vec3(0.0f, 0.0f, -speed);
    if (inputManager.IsHeld(InputAction::COW_BACKWARD))
      movement += glm::vec3(0.0f, 0.0f, speed);
    if (inputManager.IsHeld(InputAction::COW_LEFT))
      movement += glm::vec3(-speed, 0.0f, 0.0f);
    if (inputManager.IsHeld(InputAction::COW_RIGHT))
      movement += glm::vec3(speed, 0.0f, 0.0f);

    if (glm::length(movement) > 0.0f) {
      glm::vec3 newPos = cow->pos + movement;
      newPos.y = Config::Cow::GROUND_LEVEL;

      if (CollisionManager::CheckCowMovement(cow, newPos, staticModels)) {
        cow->rotation = atan2(movement.x, movement.z);
        moving = true;
      }
    }
  } else if (activeCameraIndex == Config::Scene::POV_CAMERA_INDEX) {
    if (inputManager.IsHeld(InputAction::COW_POV_MOVE)) {
      Camera *povCam = GetActiveCamera();
      glm::vec3 forward = glm::normalize(povCam->GetFront());
      glm::vec3 movement = forward * speed;
      glm::vec3 newPos = cow->pos + movement;
      newPos.y = Config::Cow::GROUND_LEVEL;

      if (CollisionManager::CheckCowMovement(cow, newPos, staticModels)) {
        moving = true;
      }
    }
  }

  auto currentAnim = animator->GetCurrentAnimation();
  if (moving) {
    Animation *walkAnim = GetWalkAnimation();
    if (walkAnim && currentAnim != walkAnim)
      animator->PlayAnimation(walkAnim);
  } else {
    Animation *idleAnim = GetIdleAnimation();
    if (idleAnim && currentAnim != idleAnim)
      animator->PlayAnimation(idleAnim);
  }

  SetAnimatedModelMoving(0, moving);
  UpdateCameraPositions();
}

/**
 * @brief Handle animation-specific input (head/tail movements).
 * @param inputManager Input manager containing current input state
 * 
 * Processes input for playing head and tail animations on separate layers:
 * - Head movements: up, down, left, right
 * - Tail movements: up
 * 
 * Animations are played as one-shot animations on specific bone layers.
 */
void Scene::HandleAnimationInput(const InputManager &inputManager) {
  if (!animationLoader)
    return;
  Animator *animator = GetFirstAnimator();
  if (!animator)
    return;

  if (inputManager.IsPressed(InputAction::HEAD_UP)) {
    auto lookUp = animationLoader->LoadAnimation(Config::Scene::ANIM_HEAD_UP);
    if (lookUp) {
      animator->PlayAnimationOnLayer(lookUp.release(), AnimationLayer::HEAD,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::HEAD_DOWN)) {
    auto lookDown =
        animationLoader->LoadAnimation(Config::Scene::ANIM_HEAD_DOWN);
    if (lookDown) {
      animator->PlayAnimationOnLayer(lookDown.release(), AnimationLayer::HEAD,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::HEAD_LEFT)) {
    auto lookLeft =
        animationLoader->LoadAnimation(Config::Scene::ANIM_HEAD_LEFT);
    if (lookLeft) {
      animator->PlayAnimationOnLayer(lookLeft.release(), AnimationLayer::HEAD,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::HEAD_RIGHT)) {
    auto lookRight =
        animationLoader->LoadAnimation(Config::Scene::ANIM_HEAD_RIGHT);
    if (lookRight) {
      animator->PlayAnimationOnLayer(lookRight.release(), AnimationLayer::HEAD,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::TAIL_UP)) {
    auto tailUp = animationLoader->LoadAnimation(Config::Scene::ANIM_TAIL_UP);
    if (tailUp) {
      animator->PlayAnimationOnLayer(tailUp.release(), AnimationLayer::TAIL,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::TAIL_LEFT)) {
    auto tailLeft =
        animationLoader->LoadAnimation(Config::Scene::ANIM_TAIL_LEFT);
    if (tailLeft) {
      animator->PlayAnimationOnLayer(tailLeft.release(), AnimationLayer::TAIL,
                                     false);
    }
  }

  if (inputManager.IsPressed(InputAction::TAIL_RIGHT)) {
    auto tailRight =
        animationLoader->LoadAnimation(Config::Scene::ANIM_TAIL_RIGHT);
    if (tailRight) {
      animator->PlayAnimationOnLayer(tailRight.release(), AnimationLayer::TAIL,
                                     false);
    }
  }
}

/**
 * @brief Update camera positions based on cow movement.
 * 
 * Updates follow and POV cameras to maintain proper positioning
 * relative to the animated cow. Follow camera stays behind the cow,
 * while POV camera shows the cow's first-person perspective.
 */
void Scene::UpdateCameraPositions() {
  AnimatedModel *cow = GetFirstAnimatedModel();
  if (!cow)
    return;

  glm::vec3 cowCenter = cow->pos + glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 upDir = Config::Camera::UP_DIR;

  if (cameras.size() > Config::Scene::FOLLOW_CAMERA_INDEX) {
    Camera *followCam = cameras[Config::Scene::FOLLOW_CAMERA_INDEX].get();
    if (followCam) {
      glm::vec3 desiredPos = cowCenter + Config::Camera::FOLLOW_OFFSET;
      followCam->SetPosition(desiredPos);
      followCam->LookAt(cowCenter, upDir);
    }
  }

  if (cameras.size() > Config::Scene::POV_CAMERA_INDEX) {
    Camera *povCam = cameras[Config::Scene::POV_CAMERA_INDEX].get();
    if (povCam) {
      glm::vec3 eyePos = cowCenter + Config::Camera::POV_EYE_OFFSET;
      povCam->SetPosition(eyePos);
    }
  }
}

/**
 * @brief Update scene state for the current frame.
 * @param deltaTime Time elapsed since last frame in seconds
 * 
 * Updates all time-dependent scene elements:
 * - Recalculates bounding boxes for static models
 * - Updates active camera (with collision detection for free camera)
 * - Updates animations and handles fallback to idle animation
 */
void Scene::Update(float deltaTime) {
  for (auto &model : staticModels) {
    model->CalculateBoundingBox();
  }

  Camera *activeCamera = GetActiveCamera();
  if (activeCamera) {
    if (activeCameraIndex == Config::Scene::FREE_CAMERA_INDEX) {
      AnimatedModel *cow = GetFirstAnimatedModel();
      if (cow)
        activeCamera->UpdateWithCollision(deltaTime, cow, staticModels);
      else
        activeCamera->Update(deltaTime);
    } else {
      activeCamera->Update(deltaTime);
    }
  }

  for (auto &obj : animatedObjects) {
    if (obj.animator) {
      obj.animator->UpdateAnimation(deltaTime);
      if (!obj.animator->GetCurrentAnimation()) {
        if (obj.idleAnimation)
          obj.animator->PlayAnimation(obj.idleAnimation.get());
      }
    }
  }
}

/**
 * @brief Render the complete scene.
 * 
 * Performs complete scene rendering by calling specialized
 * render methods for static and animated objects. Note that
 * skybox rendering is handled separately in the main render loop.
 */
void Scene::Render() {
  Camera *activeCamera = GetActiveCamera();
  if (!activeCamera)
    return;

  RenderStaticObjects();
  RenderAnimatedObjects();
}

/**
 * @brief Render all static objects in the scene.
 * 
 * Sets up the static shader with camera matrices, view position,
 * and lighting data, then renders all static models including
 * the ground plane, buildings, and objects.
 */
void Scene::RenderStaticObjects() {
  Camera *activeCamera = GetActiveCamera();
  if (!activeCamera || !staticShader)
    return;

  staticShader->Activate();
  staticShader->SetMat4(Config::Shader::UNIFORM_VP,
                        activeCamera->GetViewProjectionMatrix());
  staticShader->Set3Float(Config::Shader::UNIFORM_VIEW_POS,
                          activeCamera->GetPosition());
  SetupLighting(*staticShader);

  for (auto &model : staticModels) {
    model->Render(*staticShader);
  }
}

/**
 * @brief Render all animated objects in the scene.
 * 
 * Sets up the animated shader with camera matrices, view position,
 * lighting data, and bone transformation matrices, then renders
 * all animated models with skeletal animation support.
 */
void Scene::RenderAnimatedObjects() {
  Camera *activeCamera = GetActiveCamera();
  if (!activeCamera || !animatedShader)
    return;

  animatedShader->Activate();
  animatedShader->SetMat4(Config::Shader::UNIFORM_VP,
                          activeCamera->GetViewProjectionMatrix());
  animatedShader->Set3Float(Config::Shader::UNIFORM_VIEW_POS,
                            activeCamera->GetPosition());
  SetupLighting(*animatedShader);

  for (auto &obj : animatedObjects) {
    if (!obj.model || !obj.animator)
      continue;

    auto transforms = obj.animator->GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i) {
      animatedShader->SetMat4(std::string(Config::Shader::UNIFORM_BONES) + "[" +
                                  std::to_string(i) + "]",
                              transforms[i]);
    }
    obj.model->Render(*animatedShader);
  }
}

/**
 * @brief Configure lighting uniforms in a shader.
 * @param shader Shader to configure with lighting data
 * 
 * Sets up directional light, all point lights with their count,
 * and spot light count (currently 0) in the shader uniforms.
 */
void Scene::SetupLighting(Shader &shader) {
  directionalLight.Render(shader);

  shader.SetInt(Config::Shader::UNIFORM_POINT_CNT,
                static_cast<int>(pointLights.size()));
  for (size_t i = 0; i < pointLights.size(); ++i) {
    pointLights[i].Render(shader, static_cast<int>(i));
  }

  shader.SetInt(Config::Shader::UNIFORM_SPOT_CNT, 0);
}

/**
 * @brief Get pointer to the currently active camera.
 * @return Pointer to active camera, or nullptr if index is invalid
 */
Camera *Scene::GetActiveCamera() {
  if (activeCameraIndex < cameras.size()) {
    return cameras[activeCameraIndex].get();
  }
  return nullptr;
}

/**
 * @brief Switch to the next camera in the sequence.
 * 
 * Cycles through available cameras in order, wrapping to the first
 * camera after the last one. Logs the camera switch to console.
 */
void Scene::NextCamera() {
  if (cameras.empty())
    return;

  activeCameraIndex = (activeCameraIndex + 1) % cameras.size();
  std::cout << Config::Scene::LOG_CAM_SWITCH << activeCameraIndex << std::endl;
}

/**
 * @brief Set the active camera by index.
 * @param index Index of the camera to activate
 * 
 * Does nothing if the index is out of range.
 */
void Scene::SetActiveCamera(size_t index) {
  if (index < cameras.size()) {
    activeCameraIndex = index;
  }
}

/**
 * @brief Get camera by index.
 * @param index Index of the camera to retrieve
 * @return Pointer to camera, or nullptr if index is out of range
 */
Camera *Scene::GetCamera(size_t index) {
  if (index < cameras.size()) {
    return cameras[index].get();
  }
  return nullptr;
}

/**
 * @brief Get the total number of cameras in the scene.
 * @return Number of available cameras
 */
size_t Scene::GetCameraCount() const { return cameras.size(); }

/**
 * @brief Get the first animated model in the scene.
 * @return Pointer to first animated model, or nullptr if none exists
 */
AnimatedModel *Scene::GetFirstAnimatedModel() {
  if (!animatedObjects.empty())
    return animatedObjects[0].model.get();
  return nullptr;
}

/**
 * @brief Get the animator for the first animated object.
 * @return Pointer to first animator, or nullptr if none exists
 */
Animator *Scene::GetFirstAnimator() {
  if (!animatedObjects.empty())
    return animatedObjects[0].animator.get();
  return nullptr;
}

/**
 * @brief Get the idle animation for an animated object.
 * @param index Index of the animated object
 * @return Pointer to idle animation, or nullptr if not found
 */
Animation *Scene::GetIdleAnimation(size_t index) {
  if (index < animatedObjects.size())
    return animatedObjects[index].idleAnimation.get();
  return nullptr;
}

/**
 * @brief Get the walk animation for an animated object.
 * @param index Index of the animated object
 * @return Pointer to walk animation, or nullptr if not found
 */
Animation *Scene::GetWalkAnimation(size_t index) {
  if (index < animatedObjects.size())
    return animatedObjects[index].walkAnimation.get();
  return nullptr;
}

/**
 * @brief Get the run animation for an animated object.
 * @param index Index of the animated object
 * @return Pointer to run animation, or nullptr if not found
 */
Animation *Scene::GetRunAnimation(size_t index) {
  if (index < animatedObjects.size())
    return animatedObjects[index].runAnimation.get();
  return nullptr;
}

/**
 * @brief Set the movement state of an animated model.
 * @param index Index of the animated object
 * @param moving Whether the model is currently moving
 * 
 * Automatically switches between idle and walk animations based on
 * the movement state. Only changes animation if different from current.
 */
void Scene::SetAnimatedModelMoving(size_t index, bool moving) {
  if (index >= animatedObjects.size())
    return;

  auto &obj = animatedObjects[index];
  Animation *targetAnim =
      moving ? obj.walkAnimation.get() : obj.idleAnimation.get();
  if (targetAnim && obj.animator->GetCurrentAnimation() != targetAnim) {
    obj.animator->PlayAnimation(targetAnim);
  }
}

/**
 * @brief Set the scene's directional light properties.
 * @param light DirectionalLight structure with light parameters
 */
void Scene::SetDirectionalLight(const DirectionalLight &light) {
  directionalLight = light;
}

/**
 * @brief Add a point light to the scene.
 * @param light PointLight structure with light parameters
 */
void Scene::AddPointLight(const PointLight &light) {
  pointLights.push_back(light);
}
