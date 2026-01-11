/**
 * @file application.hpp
 * @brief Defines the Application class - main entry point for the 3D engine
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "scene.hpp"
#include "menu_system.hpp"
#include "input_manager.hpp"
#include "config.hpp"

/**
 * @class Application
 * @brief Main application class managing window, rendering, and game systems
 *
 * Handles GLFW/OpenGL initialization, window management, input processing,
 * and coordinates all major subsystems including scene rendering,
 * menu system, and input management.
 */
class Application {
public:
  /**
   * @brief Constructor - initializes all systems
   */
  Application();

  /**
   * @brief Destructor - cleanup resources
   */
  ~Application();

  /**
   * @brief Get current window width
   * @return Width in pixels
   */
  int GetWindowWidth() const;

  /**
   * @brief Get current window height
   * @return Height in pixels
   */
  int GetWindowHeight() const;

  /**
   * @brief Get GLFW window handle
   * @return Pointer to GLFW window
   */
  GLFWwindow *GetWindow() const;

  /**
   * @brief Main application loop
   */
  void Run();

private:
  /**
   * @brief Initialize GLFW library
   * @return True on success
   */
  bool InitGLFW();

  /**
   * @brief Create application window
   * @return True on success
   */
  bool InitWindow();

  /**
   * @brief Initialize GLAD OpenGL loader
   * @return True on success
   */
  bool InitGLAD();

  /**
   * @brief Set OpenGL parameters
   */
  void InitParameters();

  /**
   * @brief Register GLFW callbacks
   */
  void SetCallBacks();

  /**
   * @brief Clear framebuffer for new frame
   */
  void ClearWindow();

  /**
   * @brief Shutdown GLFW and cleanup
   */
  void Terminate();

  /**
   * @brief Process user input
   * @param deltaTime Frame time in seconds
   */
  void ProcessInput(float deltaTime);

  /**
   * @brief Window resize callback
   * @param window GLFW window handle
   * @param width New width
   * @param height New height
   */
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);

  /**
   * @brief Keyboard input callback
   * @param window GLFW window handle
   * @param key Key code
   * @param scancode System-specific scancode
   * @param action Press/release/repeat
   * @param mods Modifier keys
   */
  static void KeyboardCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods);

  /**
   * @brief Mouse movement callback
   * @param window GLFW window handle
   * @param xpos Mouse X position
   * @param ypos Mouse Y position
   */
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);

  /**
   * @brief Mouse button callback
   * @param window GLFW window handle
   * @param button Mouse button
   * @param action Press/release
   * @param mods Modifier keys
   */
  static void MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);

  GLFWwindow *window = nullptr;                      ///< GLFW window handle
  int windowWidth = Config::Window::DEFAULT_WIDTH;   ///< Current window width
  int windowHeight = Config::Window::DEFAULT_HEIGHT; ///< Current window height

  std::unique_ptr<Scene> scene;               ///< 3D scene manager
  std::unique_ptr<MenuSystem> menuSystem;     ///< ImGui menu system
  std::unique_ptr<InputManager> inputManager; ///< Input handling system
};

#endif
