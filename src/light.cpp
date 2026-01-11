/**
 * @file light.cpp
 * @brief Implementation of point, directional, and spot light rendering.
 * 
 * This file implements the rendering methods for all light types used in
 * the 3D graphics engine. Each light type handles setting appropriate
 * shader uniforms for physically-based lighting calculations.
 */

#include "light.hpp"
#include "shader.hpp"
#include <string>

// ================================================================================
// PointLight Implementation
// ================================================================================

/**
 * @brief Render point light by setting shader uniforms.
 * 
 * Sets all necessary shader uniforms for a point light at the specified
 * index in the point lights array. Includes position, attenuation factors,
 * and color/intensity values for ambient, diffuse, and specular lighting.
 * 
 * @param shader Shader program to receive the light uniforms
 * @param idx Index of this light in the pointLights array uniform
 */
void PointLight::Render(Shader &shader, int idx) {
  // Build uniform name: "pointLights[idx].property"
  std::string name = "pointLights[" + std::to_string(idx) + "]";

  // Position and attenuation parameters
  shader.Set3Float(name + ".position", this->position);
  shader.SetFloat(name + ".k0", this->k0);  // Constant attenuation
  shader.SetFloat(name + ".k1", this->k1);  // Linear attenuation
  shader.SetFloat(name + ".k2", this->k2);  // Quadratic attenuation

  // Light color components
  shader.Set4Float(name + ".ambient", this->ambient);
  shader.Set4Float(name + ".diffuse", this->diffuse);
  shader.Set4Float(name + ".specular", this->specular);
}

// ================================================================================
// DirectionalLight Implementation
// ================================================================================

/**
 * @brief Render directional light by setting shader uniforms.
 * 
 * Sets shader uniforms for a directional light (like sunlight). Directional
 * lights have no position - only a direction vector and color intensities.
 * Simulates light sources at infinite distance.
 * 
 * @param shader Shader program to receive the light uniforms
 */
void DirectionalLight::Render(Shader &shader) {
  // Uniform names fixed as "directionalLight.property"
  std::string name = "directionalLight";

  // Direction vector (normalized)
  shader.Set3Float(name + ".direction", this->direction);
  
  // Light color components
  shader.Set4Float(name + ".ambient", this->ambient);
  shader.Set4Float(name + ".diffuse", this->diffuse);
  shader.Set4Float(name + ".specular", this->specular);
}

// ================================================================================
// SpotLight Implementation
// ================================================================================

/**
 * @brief Render spot light by setting shader uniforms.
 * 
 * Sets all necessary shader uniforms for a spot light at the specified
 * index. Includes position, direction, cone angles, attenuation factors,
 * and color intensities. Creates cone-shaped illumination area.
 * 
 * @param shader Shader program to receive the light uniforms
 * @param idx Index of this light in the spotLights array uniform
 */
void SpotLight::Render(Shader &shader, int idx) {
  // Build uniform name: "spotLights[idx].property"
  std::string name = "spotLights[" + std::to_string(idx) + "]";

  // Position and direction
  shader.Set3Float(name + ".position", this->position);
  shader.Set3Float(name + ".direction", this->direction);

  // Cone parameters (in radians)
  shader.SetFloat(name + ".cutOff", this->cutOff);        // Inner cone angle
  shader.SetFloat(name + ".outerCutOff", this->outerCutOff); // Outer cone angle

  // Attenuation parameters
  shader.SetFloat(name + ".k0", this->k0);  // Constant attenuation
  shader.SetFloat(name + ".k1", this->k1);  // Linear attenuation
  shader.SetFloat(name + ".k2", this->k2);  // Quadratic attenuation

  // Light color components
  shader.Set4Float(name + ".ambient", this->ambient);
  shader.Set4Float(name + ".diffuse", this->diffuse);
  shader.Set4Float(name + ".specular", this->specular);
}
