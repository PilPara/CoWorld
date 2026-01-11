// #include "camera.hpp"
// #include "animated_model.hpp"
// #include "model.hpp"
// #include <algorithm>
//
// Camera::Camera(const glm::vec3 &pos, const glm::vec3 &target,
//                const glm::vec3 &up, PersProjInfo &persProjInfo) {
//   this->cameraPos = pos;
//   this->up = up;
//
//   float aspectRatio = (float)persProjInfo.Width / (float)persProjInfo.Height;
//
//   this->cameraOrientation = glm::lookAt(pos, target, up);
//   this->persProjection = glm::perspective(
//       persProjInfo.FOV, aspectRatio, persProjInfo.zNear, persProjInfo.zFar);
// }
//
// glm::mat4 Camera::GetProjectionMatrix() const { return this->persProjection;
// }
//
// glm::vec3 Camera::GetPosition() const { return this->cameraPos; }
//
// glm::mat4 Camera::GetViewMatrix() const {
//   glm::mat4 translation = glm::translate(glm::mat4(1.0f), -this->cameraPos);
//   glm::mat4 rotation = glm::mat4_cast(this->cameraOrientation);
//   return rotation * translation;
// }
//
// glm::vec3 Camera::GetFront() const {
//   glm::mat4 V = glm::mat4_cast(this->cameraOrientation);
//   return glm::normalize(-glm::vec3(V[0][2], V[1][2], V[2][2]));
// }
//
// glm::mat4 Camera::GetViewProjectionMatrix() const {
//   glm::mat4 view = GetViewMatrix();
//   return this->persProjection * view;
// }
//
// void Camera::Update(float dt) {
//   CalculateCameraOrientation();
//   this->oldMousePos = this->mouseState.pos;
//
//   CalculateVelocity(dt);
//   this->cameraPos += this->velocity * dt;
// }
//
// void Camera::UpdateWithCollision(
//     float dt, const AnimatedModel *cow,
//     const std::vector<std::unique_ptr<Model>> &staticModels) {
//
//   CalculateCameraOrientation();
//   this->oldMousePos = this->mouseState.pos;
//
//   CalculateVelocity(dt);
//   glm::vec3 newPos = this->cameraPos + this->velocity * dt;
//
//   // Test collision before moving
//   if (CollisionManager::CheckCameraMovement(newPos, cow, staticModels)) {
//     this->cameraPos = newPos;
//   } else {
//     // Reduce velocity when hitting obstacles to prevent jittering
//     this->velocity *= 0.1f;
//   }
// }
//
// void Camera::SetPosition(const glm::vec3 &pos) { this->cameraPos = pos; }
//
// void Camera::LookAt(const glm::vec3 &target, const glm::vec3 &up) {
//   this->cameraOrientation =
//       glm::quat_cast(glm::lookAt(this->cameraPos, target, up));
//   this->up = up;
// }
//
// void Camera::CalculateCameraOrientation() {
//   glm::vec2 deltaMouse = this->mouseState.pos - this->oldMousePos;
//   glm::quat deltaQuat = glm::quat(glm::vec3(
//       this->mouseSpeed * deltaMouse.y, this->mouseSpeed * deltaMouse.x,
//       0.0f));
//
//   this->cameraOrientation = glm::normalize(deltaQuat *
//   this->cameraOrientation); SetUpVector();
// }
//
// void Camera::SetUpVector() {
//   glm::mat4 view = GetViewMatrix();
//   glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);
//   this->cameraOrientation =
//       glm::lookAt(this->cameraPos, this->cameraPos - forward, this->up);
// }
//
// glm::vec3 Camera::CalculateAcceleration() {
//   glm::mat4 V = glm::mat4_cast(this->cameraOrientation);
//   glm::vec3 right = glm::vec3(V[0][0], V[1][0], V[2][0]);
//   glm::vec3 forward = -glm::vec3(V[0][2], V[1][2], V[2][2]);
//   glm::vec3 up = glm::cross(right, forward);
//
//   glm::vec3 acceleration = glm::vec3(0.0f);
//
//   if (this->movement.Forward)
//     acceleration += forward;
//   if (this->movement.Backward)
//     acceleration += -forward;
//   if (this->movement.StrafeRight)
//     acceleration += right;
//   if (this->movement.StrafeLeft)
//     acceleration -= right;
//   if (this->movement.Up)
//     acceleration += up;
//   if (this->movement.Down)
//     acceleration -= up;
//   if (this->movement.Fast)
//     acceleration *= this->fastCoef;
//
//   return acceleration;
// }
//
// void Camera::CalculateVelocity(float dt) {
//   glm::vec3 acceleration = CalculateAcceleration();
//
//   if (acceleration == glm::vec3(0.0f)) {
//     this->velocity -= this->velocity * std::min(dt * this->damping, 1.0f);
//   } else {
//     this->velocity = acceleration * this->acceleration * dt;
//     float maxSpeed =
//         this->movement.Fast ? this->maxSpeed * this->fastCoef :
//         this->maxSpeed;
//   }
//
//   if (glm::length(this->velocity) >= this->maxSpeed) {
//     this->velocity = glm::normalize(this->velocity) * this->maxSpeed;
//   }
// }

/**
 * @file camera.cpp
 * @brief Implementation of the Camera class for free-fly and follow POV
 * cameras.
 */

#include "camera.hpp"
#include "animated_model.hpp"
#include "model.hpp"
#include <algorithm>

// -------------------- Constructor --------------------

Camera::Camera(const glm::vec3 &pos, const glm::vec3 &target,
               const glm::vec3 &up, PersProjInfo &persProjInfo) {
  this->cameraPos = pos;
  this->up = up;

  // Calculate aspect ratio
  float aspectRatio = (float)persProjInfo.Width / (float)persProjInfo.Height;

  // Initial orientation and projection matrix
  this->cameraOrientation = glm::lookAt(pos, target, up);
  this->persProjection = glm::perspective(
      persProjInfo.FOV, aspectRatio, persProjInfo.zNear, persProjInfo.zFar);
}

// -------------------- Getters --------------------

glm::mat4 Camera::GetProjectionMatrix() const { return this->persProjection; }
glm::vec3 Camera::GetPosition() const { return this->cameraPos; }

glm::mat4 Camera::GetViewMatrix() const {
  // Build view matrix from orientation and position
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), -this->cameraPos);
  glm::mat4 rotation = glm::mat4_cast(this->cameraOrientation);
  return rotation * translation;
}

glm::vec3 Camera::GetFront() const {
  // Extract forward vector from orientation quaternion
  glm::mat4 V = glm::mat4_cast(this->cameraOrientation);
  return glm::normalize(-glm::vec3(V[0][2], V[1][2], V[2][2]));
}

glm::mat4 Camera::GetViewProjectionMatrix() const {
  // Combine projection and view for rendering
  return this->persProjection * GetViewMatrix();
}

// -------------------- Updates --------------------

void Camera::Update(float dt) {
  // Update camera orientation from mouse input
  CalculateCameraOrientation();
  this->oldMousePos = this->mouseState.pos;

  // Update velocity and apply movement
  CalculateVelocity(dt);
  this->cameraPos += this->velocity * dt;
}

void Camera::UpdateWithCollision(
    float dt, const AnimatedModel *cow,
    const std::vector<std::unique_ptr<Model>> &staticModels) {

  CalculateCameraOrientation();
  this->oldMousePos = this->mouseState.pos;

  // Predict new position
  CalculateVelocity(dt);
  glm::vec3 newPos = this->cameraPos + this->velocity * dt;

  // Test collision before moving
  if (CollisionManager::CheckCameraMovement(newPos, cow, staticModels)) {
    this->cameraPos = newPos;
  } else {
    // If collision → slow down drastically
    this->velocity *= 0.1f;
  }
}

// -------------------- Positioning --------------------

void Camera::SetPosition(const glm::vec3 &pos) { this->cameraPos = pos; }

void Camera::LookAt(const glm::vec3 &target, const glm::vec3 &up) {
  // Recalculate orientation towards target
  this->cameraOrientation =
      glm::quat_cast(glm::lookAt(this->cameraPos, target, up));
  this->up = up;
}

// -------------------- Orientation helpers --------------------

void Camera::CalculateCameraOrientation() {
  // Mouse delta → rotation quaternion
  glm::vec2 deltaMouse = this->mouseState.pos - this->oldMousePos;
  glm::quat deltaQuat = glm::quat(glm::vec3(
      this->mouseSpeed * deltaMouse.y, this->mouseSpeed * deltaMouse.x, 0.0f));

  // Apply rotation
  this->cameraOrientation = glm::normalize(deltaQuat * this->cameraOrientation);

  // Ensure up vector remains consistent
  SetUpVector();
}

void Camera::SetUpVector() {
  // Rebuild orientation to keep world-up consistent
  glm::mat4 view = GetViewMatrix();
  glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);
  this->cameraOrientation =
      glm::lookAt(this->cameraPos, this->cameraPos - forward, this->up);
}

// -------------------- Movement helpers --------------------

glm::vec3 Camera::CalculateAcceleration() {
  // Basis vectors from orientation
  glm::mat4 V = glm::mat4_cast(this->cameraOrientation);
  glm::vec3 right = glm::vec3(V[0][0], V[1][0], V[2][0]);
  glm::vec3 forward = -glm::vec3(V[0][2], V[1][2], V[2][2]);
  glm::vec3 up = glm::cross(right, forward);

  glm::vec3 acceleration = glm::vec3(0.0f);

  // Combine acceleration based on input state
  if (this->movement.Forward)
    acceleration += forward;
  if (this->movement.Backward)
    acceleration += -forward;
  if (this->movement.StrafeRight)
    acceleration += right;
  if (this->movement.StrafeLeft)
    acceleration -= right;
  if (this->movement.Up)
    acceleration += up;
  if (this->movement.Down)
    acceleration -= up;

  // Boost if fast key is held
  if (this->movement.Fast)
    acceleration *= this->fastCoef;

  return acceleration;
}

void Camera::CalculateVelocity(float dt) {
  glm::vec3 acceleration = CalculateAcceleration();

  if (acceleration == glm::vec3(0.0f)) {
    // Apply damping when no input → slows to stop
    this->velocity -= this->velocity * std::min(dt * this->damping, 1.0f);
  } else {
    // Accelerate in input direction
    this->velocity = acceleration * this->acceleration * dt;
  }

  // Clamp to max speed
  if (glm::length(this->velocity) >= this->maxSpeed) {
    this->velocity = glm::normalize(this->velocity) * this->maxSpeed;
  }
}
