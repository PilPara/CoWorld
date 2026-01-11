/**
 * @file camera.hpp
 * @brief Declaration of the Camera class for free-fly, follow, and POV cameras.
 *
 * Provides perspective projection, view matrix generation, mouse-look controls,
 * WASD-like movement, and collision-aware updates with the scene.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include "config.hpp"

/**
 * @brief Perspective projection parameters
 */
struct PersProjInfo {
  float FOV;    ///< Field of view in radians
  float Width;  ///< Viewport width in pixels
  float Height; ///< Viewport height in pixels
  float zNear;  ///< Near clipping plane
  float zFar;   ///< Far clipping plane
};

/**
 * @brief Camera class supporting movement, orientation, and collisions.
 */
class Camera {
public:
  /**
   * @brief Construct a camera with position, target, and projection info.
   * @param pos Starting position of the camera
   * @param target Point camera looks at initially
   * @param up World up vector
   * @param persProjInfo Projection parameters (FOV, aspect ratio, near/far)
   */
  Camera(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up,
         PersProjInfo &persProjInfo);

  // ---- Matrix accessors ----
  glm::mat4 GetProjectionMatrix() const;     ///< Get projection matrix
  glm::vec3 GetPosition() const;             ///< Get camera position
  glm::mat4 GetViewMatrix() const;           ///< Get view matrix
  glm::vec3 GetFront() const;                ///< Get forward direction vector
  glm::mat4 GetViewProjectionMatrix() const; ///< Get combined VP matrix

  // ---- Updates ----
  void Update(float dt); ///< Update without collision
  void UpdateWithCollision(
      float dt, const class AnimatedModel *cow,
      const std::vector<std::unique_ptr<class Model>> &staticModels);

  // ---- Positioning ----
  void SetPosition(const glm::vec3 &pos); ///< Manually set position
  void LookAt(const glm::vec3 &target,
              const glm::vec3 &up); ///< Re-orient camera to target

  // ---- Input state ----

  /**
   * @brief Tracks mouse input state for orientation.
   */
  struct MouseState {
    glm::vec2 pos{0.0f};        ///< Current mouse position (normalized coords)
    bool buttonPressed = false; ///< Whether mouse button is held
  } mouseState;

  /**
   * @brief Tracks movement input (keyboard).
   *
   * Booleans are set externally (e.g., via input manager).
   */
  struct Movement {
    bool Forward = false;     ///< Move forward
    bool Backward = false;    ///< Move backward
    bool StrafeLeft = false;  ///< Move left (A)
    bool StrafeRight = false; ///< Move right (D)
    bool Up = false;          ///< Move upward
    bool Down = false;        ///< Move downward
    bool Fast = false;        ///< Boost speed
  } movement;

private:
  // ---- Core state ----
  glm::vec3 cameraPos;         ///< Current position in world space
  glm::quat cameraOrientation; ///< Current orientation (rotation)
  glm::vec3 up;                ///< Current up vector
  glm::mat4 persProjection;    ///< Projection matrix

  // ---- Motion state ----
  glm::vec3 velocity{0.0f};    ///< Current velocity
  glm::vec2 oldMousePos{0.0f}; ///< Previous mouse pos (for delta calc)

  // ---- Tuning constants ----
  float mouseSpeed = Config::Camera::MOUSE_SENSITIVITY; ///< Mouse sensitivity
  float acceleration = Config::Camera::ACCELERATION;    ///< Acceleration factor
  float damping = Config::Camera::DAMPING;    ///< Damping factor (slowdown)
  float maxSpeed = Config::Camera::MAX_SPEED; ///< Maximum speed
  float fastCoef =
      Config::Camera::FAST_COEF; ///< Speed multiplier for fast mode

  // ---- Internal helpers ----
  void CalculateCameraOrientation(); ///< Update orientation from mouse delta
  void SetUpVector();                ///< Keep up vector consistent
  glm::vec3 CalculateAcceleration(); ///< Compute accel from input state
  void CalculateVelocity(float dt);  ///< Update velocity from accel
};

#endif
