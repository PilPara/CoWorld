/**
 * @file input_manager.cpp
 * @brief Implementation of the InputManager class for keyboard and mouse handling.
 * 
 * Provides a high-level input abstraction over GLFW, mapping physical keys
 * to game actions and tracking input states. Handles both keyboard and mouse
 * input with frame-based state tracking for pressed, held, and released events.
 */

#include "input_manager.hpp"
#include <glm/glm.hpp>
#include <iostream>

// ================================================================================
// Constructor and Initialization
// ================================================================================

/**
 * @brief Constructor - initialize input manager with GLFW window.
 * 
 * Sets up key mappings, initializes mouse position tracking, and prepares
 * input state arrays for frame-based input detection.
 * 
 * @param window GLFW window handle for input polling
 */
InputManager::InputManager(GLFWwindow *window) : window(window) {
  InitializeKeyMappings();

  // Get initial mouse position to prevent large delta on first frame
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mousePosition = glm::vec2(xpos, ypos);
  lastMousePosition = mousePosition;
}

// ================================================================================
// Key Mapping Configuration
// ================================================================================

/**
 * @brief Initialize the mapping from GLFW keys to game actions.
 * 
 * Configures the complete key mapping table that translates physical
 * keyboard keys to logical game actions. This separation allows for
 * easy key remapping and action-based input handling.
 */
void InputManager::InitializeKeyMappings() {
  // ---- System Controls ----
  keyMappings[GLFW_KEY_ESCAPE] = InputAction::QUIT;
  keyMappings[GLFW_KEY_M] = InputAction::TOGGLE_MENU;

  // ---- Camera Movement ----
  keyMappings[GLFW_KEY_W] = InputAction::CAMERA_FORWARD;
  keyMappings[GLFW_KEY_S] = InputAction::CAMERA_BACKWARD;
  keyMappings[GLFW_KEY_A] = InputAction::CAMERA_LEFT;
  keyMappings[GLFW_KEY_D] = InputAction::CAMERA_RIGHT;
  keyMappings[GLFW_KEY_E] = InputAction::CAMERA_UP;
  keyMappings[GLFW_KEY_Q] = InputAction::CAMERA_DOWN;
  keyMappings[GLFW_KEY_F] = InputAction::CAMERA_FAST;      // Speed boost
  keyMappings[GLFW_KEY_TAB] = InputAction::NEXT_CAMERA;    // Cycle cameras
  keyMappings[GLFW_KEY_R] = InputAction::RESET_CAMERA;     // Reset position

  // ---- Cow Movement (Arrow Keys) ----
  keyMappings[GLFW_KEY_UP] = InputAction::COW_FORWARD;
  keyMappings[GLFW_KEY_DOWN] = InputAction::COW_BACKWARD;
  keyMappings[GLFW_KEY_LEFT] = InputAction::COW_LEFT;
  keyMappings[GLFW_KEY_RIGHT] = InputAction::COW_RIGHT;
  keyMappings[GLFW_KEY_SPACE] = InputAction::COW_POV_MOVE; // POV movement mode

  // ---- Animation Controls ----
  keyMappings[GLFW_KEY_1] = InputAction::HEAD_UP;
  keyMappings[GLFW_KEY_2] = InputAction::HEAD_DOWN;
  keyMappings[GLFW_KEY_3] = InputAction::HEAD_LEFT;
  keyMappings[GLFW_KEY_4] = InputAction::HEAD_RIGHT;
  keyMappings[GLFW_KEY_Z] = InputAction::TAIL_UP;
  keyMappings[GLFW_KEY_X] = InputAction::TAIL_LEFT;
  keyMappings[GLFW_KEY_C] = InputAction::TAIL_RIGHT;
}

// ================================================================================
// Frame-Based Input Processing
// ================================================================================

/**
 * @brief Update input states for the current frame.
 * 
 * Must be called once per frame to update input states. Calculates mouse
 * movement delta, resets single-frame events (pressed/released), and polls
 * all mapped keys to update their current state.
 */
void InputManager::Update() {
  // Calculate mouse movement since last frame
  mouseDelta = mousePosition - lastMousePosition;
  lastMousePosition = mousePosition;

  // Reset single-frame events for next frame
  for (auto &[action, state] : inputStates) {
    state.pressed = false;  // Only true for one frame when key first pressed
    state.released = false; // Only true for one frame when key released
  }

  // Poll all mapped keys and update their states
  for (const auto &[key, action] : keyMappings) {
    bool currentPressed = glfwGetKey(window, key) == GLFW_PRESS;
    UpdateInputState(action, currentPressed);
  }
}

/**
 * @brief Update the state of a specific input action.
 * 
 * Handles state transitions for pressed/held/released detection.
 * Ensures proper single-frame event firing for UI and gameplay logic.
 * 
 * @param action Game action to update
 * @param currentPressed Whether the key is currently pressed this frame
 */
void InputManager::UpdateInputState(InputAction action, bool currentPressed) {
  InputState &state = inputStates[action];

  // Handle state transitions
  if (currentPressed && !state.held) {
    // Key just pressed this frame
    state.pressed = true;
    state.held = true;
  } else if (!currentPressed && state.held) {
    // Key just released this frame
    state.released = true;
    state.held = false;
  }
  // Note: held state persists until key is released
}

// ================================================================================
// Input State Queries
// ================================================================================

/**
 * @brief Check if an action was just pressed this frame.
 * @param action Game action to check
 * @return True only on the first frame the key was pressed
 */
bool InputManager::IsPressed(InputAction action) const {
  auto it = inputStates.find(action);
  return it != inputStates.end() ? it->second.pressed : false;
}

/**
 * @brief Check if an action is currently being held.
 * @param action Game action to check
 * @return True while the key remains pressed
 */
bool InputManager::IsHeld(InputAction action) const {
  auto it = inputStates.find(action);
  return it != inputStates.end() ? it->second.held : false;
}

/**
 * @brief Check if an action was just released this frame.
 * @param action Game action to check
 * @return True only on the first frame the key was released
 */
bool InputManager::IsReleased(InputAction action) const {
  auto it = inputStates.find(action);
  return it != inputStates.end() ? it->second.released : false;
}

// ================================================================================
// Mouse Input Access
// ================================================================================

/**
 * @brief Get current absolute mouse position.
 * @return Mouse position in screen coordinates
 */
glm::vec2 InputManager::GetMousePosition() const { return mousePosition; }

/**
 * @brief Get mouse movement delta since last frame.
 * @return Mouse movement vector (useful for camera controls)
 */
glm::vec2 InputManager::GetMouseDelta() const { return mouseDelta; }

/**
 * @brief Check if a mouse button was pressed this frame.
 * @param button GLFW mouse button constant (GLFW_MOUSE_BUTTON_LEFT, etc.)
 * @return True only on the frame the button was pressed
 */
bool InputManager::IsMouseButtonPressed(int button) const {
  auto it = mouseButtons.find(button);
  return it != mouseButtons.end() ? it->second : false;
}

/**
 * @brief Check if a mouse button is currently held down.
 * @param button GLFW mouse button constant
 * @return True while the button remains pressed
 */
bool InputManager::IsMouseButtonHeld(int button) const {
  return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

// ================================================================================
// GLFW Callback Handlers
// ================================================================================

/**
 * @brief Handle GLFW key callback events.
 * 
 * Currently used for immediate key handling if needed. Continuous input
 * states are handled through the Update() polling method for consistent
 * frame-based input processing.
 * 
 * @param key GLFW key code
 * @param scancode Platform-specific scancode
 * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT
 * @param mods Modifier key flags
 */
void InputManager::OnKeyCallback(int key, int scancode, int action, int mods) {
  // Immediate key handling could be added here for special cases
  // Most input is handled through continuous Update() polling
}

/**
 * @brief Handle GLFW mouse movement callback.
 * 
 * Updates the current mouse position. Delta calculation is performed
 * in the Update() method to ensure frame-synchronized input handling.
 * 
 * @param xpos Mouse X position in screen coordinates
 * @param ypos Mouse Y position in screen coordinates
 */
void InputManager::OnMouseCallback(double xpos, double ypos) {
  // Update absolute mouse position
  mousePosition = glm::vec2(xpos, ypos);
}

/**
 * @brief Handle GLFW mouse button callback.
 * 
 * Tracks mouse button press/release events for single-frame detection.
 * Used alongside the held state checking for complete mouse input handling.
 * 
 * @param button GLFW mouse button constant
 * @param action GLFW_PRESS or GLFW_RELEASE
 * @param mods Modifier key flags
 */
void InputManager::OnMouseButtonCallback(int button, int action, int mods) {
  // Track mouse button pressed state for single-frame detection
  mouseButtons[button] = (action == GLFW_PRESS);
}
