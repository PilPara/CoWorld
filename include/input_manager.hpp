/**
 * @file input_manager.hpp
 * @brief Declaration of the InputManager class for keyboard and mouse handling.
 *
 * Provides an abstraction over GLFW input, mapping keys to game actions,
 * tracking pressed/held/released states, and handling mouse position/buttons.
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>

/**
 * @brief Enumerates all supported game actions.
 *
 * These map from GLFW keys → gameplay controls.
 */
enum class InputAction {
  // System
  QUIT,
  TOGGLE_MENU,

  // Camera
  CAMERA_FORWARD,
  CAMERA_BACKWARD,
  CAMERA_LEFT,
  CAMERA_RIGHT,
  CAMERA_UP,
  CAMERA_DOWN,
  CAMERA_FAST,
  NEXT_CAMERA,
  RESET_CAMERA,

  // Cow movement
  COW_FORWARD,
  COW_BACKWARD,
  COW_LEFT,
  COW_RIGHT,
  COW_POV_MOVE,

  // Animations
  HEAD_UP,
  HEAD_DOWN,
  HEAD_LEFT,
  HEAD_RIGHT,
  TAIL_UP,
  TAIL_LEFT,
  TAIL_RIGHT
};

/**
 * @brief Stores the state of a single input action.
 */
struct InputState {
  bool pressed = false;  ///< True for one frame when first pressed
  bool held = false;     ///< True while key is held down
  bool released = false; ///< True for one frame when released
};

/**
 * @brief Handles keyboard + mouse input using GLFW.
 *
 * Maintains mappings from keys to actions and tracks state transitions.
 */
class InputManager {
public:
  explicit InputManager(GLFWwindow *window);

  void Update(); ///< Poll states and update pressed/held/released transitions

  // Query states
  bool IsPressed(InputAction action) const;
  bool IsHeld(InputAction action) const;
  bool IsReleased(InputAction action) const;

  // Mouse accessors
  glm::vec2 GetMousePosition() const;
  glm::vec2 GetMouseDelta() const;
  bool IsMouseButtonPressed(int button) const;
  bool IsMouseButtonHeld(int button) const;

  // GLFW callback hooks (called from Application)
  void OnKeyCallback(int key, int scancode, int action, int mods);
  void OnMouseCallback(double xpos, double ypos);
  void OnMouseButtonCallback(int button, int action, int mods);

private:
  GLFWwindow *window; ///< Associated GLFW window

  glm::vec2 mousePosition;     ///< Current mouse position
  glm::vec2 lastMousePosition; ///< Last frame mouse position
  glm::vec2 mouseDelta;        ///< Per-frame delta

  std::unordered_map<int, InputAction> keyMappings; ///< Key → action mapping
  std::unordered_map<InputAction, InputState> inputStates; ///< Action → state
  std::unordered_map<int, bool> mouseButtons; ///< Mouse button state

  void InitializeKeyMappings(); ///< Setup default key → action bindings
  void UpdateInputState(InputAction action, bool currentPressed);
};

#endif
