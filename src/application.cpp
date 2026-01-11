/**
 * @file application.cpp
 * @brief Implementation of the Application class - main application controller
 */

#include "application.hpp"
#include "config.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

/**
 * @brief Constructor - initializes all application systems
 *
 * Sets up OpenGL context, creates window, initializes subsystems.
 * Order is critical: GLFW -> Window -> GLAD -> Systems
 */
Application::Application() {
  if (!InitGLFW() || !InitWindow() || !InitGLAD()) {
    Terminate();
    return;
  }

  InitParameters();

  // Initialize core systems
  this->inputManager = std::make_unique<InputManager>(this->window);
  this->menuSystem = std::make_unique<MenuSystem>();
  this->menuSystem->Initialize(this->window);

  // Set callbacks AFTER initializing MenuSystem
  SetCallBacks();

  // Create and initialize scene
  this->scene = std::make_unique<Scene>();
  this->scene->Initialize(windowWidth, windowHeight);
}

/**
 * @brief Destructor - cleanup all resources
 */
Application::~Application() {
  if (menuSystem) {
    menuSystem->Shutdown();
  }
  Terminate();
}

/**
 * @brief Get current window width
 * @return Width in pixels
 */
int Application::GetWindowWidth() const { return this->windowWidth; }

/**
 * @brief Get current window height
 * @return Height in pixels
 */
int Application::GetWindowHeight() const { return this->windowHeight; }

/**
 * @brief Get GLFW window handle
 * @return Pointer to GLFW window
 */
GLFWwindow *Application::GetWindow() const { return this->window; }

/**
 * @brief Main application loop
 *
 * Handles frame timing, input processing, scene updates,
 * and rendering. Runs until window close is requested.
 */
void Application::Run() {
  float lastFrame = glfwGetTime();

  while (!glfwWindowShouldClose(this->window)) {
    // Poll window events
    glfwPollEvents();

    // Calculate delta time
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update input manager
    inputManager->Update();

    // Process input
    ProcessInput(deltaTime);

    // Update scene
    if (this->scene) {
      this->scene->Update(deltaTime);
    }

    // Begin frame rendering
    ClearWindow();

    // Scene rendering
    if (this->scene) {
      this->scene->Render();

      // Render skybox
      if (Camera *cam = this->scene->GetActiveCamera()) {
        if (Skybox *skybox = this->scene->GetSkybox()) {
          skybox->Render(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
      }
    }

    // Render menu system
    if (menuSystem) {
      menuSystem->Render(scene.get());
    }

    // Present frame
    glfwSwapBuffers(this->window);
  }
}

/**
 * @brief Process user input
 *
 * Handles system-level input and forwards game input to scene
 * when menu is not active.
 *
 * @param deltaTime Frame time for movement calculations
 */
void Application::ProcessInput(float deltaTime) {
  // System input
  if (inputManager->IsPressed(InputAction::QUIT)) {
    glfwSetWindowShouldClose(this->window, true);
  }

  if (inputManager->IsPressed(InputAction::TOGGLE_MENU)) {
    menuSystem->Toggle();
  }

  // Pass input to scene if menu is not active
  if (!menuSystem->IsActive() && scene) {
    scene->HandleInput(*inputManager, deltaTime);
  }
}

/**
 * @brief Initialize GLFW library
 *
 * Sets OpenGL version and profile requirements
 *
 * @return True on success
 */
bool Application::InitGLFW() {
  if (!glfwInit()) {
    std::cout << "[ERROR::APPLICATION::INIT] Failed to initialize GLFW"
              << std::endl;
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Config::GL::CONTEXT_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Config::GL::CONTEXT_VERSION_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, Config::GL::PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, Config::GL::FORWARD_COMPAT);

  return true;
}

/**
 * @brief Create application window
 * @return True on success
 */
bool Application::InitWindow() {
  this->window = glfwCreateWindow(Config::Window::DEFAULT_WIDTH,
                                  Config::Window::DEFAULT_HEIGHT,
                                  Config::Window::TITLE, NULL, NULL);
  if (!this->window) {
    std::cout << "[ERROR::APPLICATION::WINDOW] Failed to create GLFW window"
              << std::endl;
    return false;
  }
  glfwMakeContextCurrent(this->window);
  return true;
}

/**
 * @brief Initialize GLAD OpenGL function loader
 * @return True on success
 */
bool Application::InitGLAD() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "[ERROR::APPLICATION::GLAD] Failed to initialize GLAD"
              << std::endl;
    return false;
  }
  return true;
}

/**
 * @brief Set initial OpenGL parameters
 */
void Application::InitParameters() {
  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

/**
 * @brief Register GLFW callbacks
 *
 * Sets user pointer and registers all input callbacks
 */
void Application::SetCallBacks() {
  glfwSetWindowUserPointer(this->window, this);
  glfwSetFramebufferSizeCallback(this->window, FramebufferSizeCallback);
  glfwSetKeyCallback(this->window, KeyboardCallback);
  glfwSetCursorPosCallback(this->window, MouseCallback);
  glfwSetMouseButtonCallback(this->window, MouseButtonCallback);
}

/**
 * @brief Clear framebuffer for new frame
 */
void Application::ClearWindow() {
  glClearColor(Config::Render::CLEAR_COLOR_R, Config::Render::CLEAR_COLOR_G,
               Config::Render::CLEAR_COLOR_B, Config::Render::CLEAR_COLOR_A);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Terminate GLFW
 */
void Application::Terminate() {
  if (this->window) {
    glfwTerminate();
  }
}

/**
 * @brief Handle window resize
 * @param window GLFW window
 * @param width New width
 * @param height New height
 */
void Application::FramebufferSizeCallback(GLFWwindow *window, int width,
                                          int height) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app) {
    app->windowWidth = width;
    app->windowHeight = height;
  }
  glViewport(0, 0, width, height);
}

/**
 * @brief Handle mouse movement
 *
 * Forwards to ImGui first, then input manager and camera if appropriate
 *
 * @param window GLFW window
 * @param xpos Mouse X position
 * @param ypos Mouse Y position
 */
void Application::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  // Always forward to ImGui first
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

  // Forward to input manager
  app->inputManager->OnMouseCallback(xpos, ypos);

  // Skip game input if ImGui wants mouse
  if (ImGui::GetIO().WantCaptureMouse || app->menuSystem->IsActive()) {
    return;
  }

  // Update camera mouse state if scene is active
  if (app->scene) {
    if (Camera *camera = app->scene->GetActiveCamera()) {
      camera->mouseState.pos.x = (float)xpos / (float)app->windowWidth;
      camera->mouseState.pos.y = (float)ypos / (float)app->windowHeight;

      // Update cow rotation for POV camera
      if (app->scene->activeCameraIndex == Config::Camera::POV_INDEX) {
        if (AnimatedModel *cow = app->scene->GetFirstAnimatedModel()) {
          glm::vec3 cameraFront = camera->GetFront();
          cow->rotation = atan2(cameraFront.x, cameraFront.z);
        }
      }
    }
  }
}

/**
 * @brief Handle mouse button input
 *
 * Forwards to ImGui, input manager, and camera system
 *
 * @param window GLFW window
 * @param button Mouse button code
 * @param action Press/release
 * @param mods Modifier keys
 */
void Application::MouseButtonCallback(GLFWwindow *window, int button,
                                      int action, int mods) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  // Always forward to ImGui first
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

  // Forward to input manager
  app->inputManager->OnMouseButtonCallback(button, action, mods);

  // Skip game input if ImGui wants mouse
  if (ImGui::GetIO().WantCaptureMouse || app->menuSystem->IsActive()) {
    return;
  }

  // Update camera if not in menu
  if (app->scene) {
    if (Camera *camera = app->scene->GetActiveCamera()) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        camera->mouseState.buttonPressed = (action == GLFW_PRESS);
      }
    }
  }
}

/**
 * @brief Handle keyboard input
 *
 * Forwards to ImGui, input manager, and camera movement system
 *
 * @param window GLFW window
 * @param key Key code
 * @param scancode System scancode
 * @param action Press/release/repeat
 * @param mods Modifier keys
 */
void Application::KeyboardCallback(GLFWwindow *window, int key, int scancode,
                                   int action, int mods) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  // Always forward to ImGui first
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

  // Forward to input manager
  app->inputManager->OnKeyCallback(key, scancode, action, mods);

  // Skip game input if ImGui wants keyboard
  if (ImGui::GetIO().WantCaptureKeyboard || app->menuSystem->IsActive()) {
    return;
  }

  // Update camera movement state if not in menu
  if (app->scene) {
    if (Camera *camera = app->scene->GetActiveCamera()) {
      bool press = action != GLFW_RELEASE;
      switch (key) {
      case GLFW_KEY_W:
        camera->movement.Forward = press;
        break;
      case GLFW_KEY_S:
        camera->movement.Backward = press;
        break;
      case GLFW_KEY_D:
        camera->movement.StrafeRight = press;
        break;
      case GLFW_KEY_A:
        camera->movement.StrafeLeft = press;
        break;
      case GLFW_KEY_E:
        camera->movement.Up = press;
        break;
      case GLFW_KEY_Q:
        camera->movement.Down = press;
        break;
      case GLFW_KEY_F:
        camera->movement.Fast = press;
        break;
      }
    }
  }
}
