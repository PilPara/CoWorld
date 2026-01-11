/**
 * @file light.hpp
 * @brief Declarations of light structures for physically-based lighting.
 *
 * Defines point lights, directional lights, and spot lights used in the
 * 3D graphics engine. Each light type includes properties for position/direction,
 * attenuation, and Phong lighting model color components (ambient, diffuse, specular).
 * 
 * These structures interface with GLSL shaders through uniform variables
 * and support multiple lights of each type for complex scene lighting.
 */

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "shader.hpp"

/**
 * @struct PointLight
 * @brief Omnidirectional light source with distance-based attenuation.
 *
 * Represents a point light source that emits light equally in all directions
 * from a specific position. Light intensity decreases with distance according
 * to the attenuation formula: 1 / (k0 + k1*d + k2*d²), where d is distance.
 * 
 * Commonly used for: light bulbs, torches, fire effects, small area lights.
 */
struct PointLight {
  glm::vec3 position; ///< World space position of the light source

  // Attenuation coefficients for distance-based falloff
  float k0; ///< Constant attenuation factor (usually 1.0)
  float k1; ///< Linear attenuation factor
  float k2; ///< Quadratic attenuation factor

  // Phong lighting model color components
  glm::vec4 ambient;  ///< Ambient light color and intensity
  glm::vec4 diffuse;  ///< Diffuse light color and intensity
  glm::vec4 specular; ///< Specular light color and intensity

  /**
   * @brief Send light parameters to shader as uniform array element.
   * @param shader Target shader program with pointLights uniform array
   * @param idx Index in the pointLights array (0-based)
   */
  void Render(Shader &shader, int idx);
};

/**
 * @struct DirectionalLight
 * @brief Infinite distance light source with uniform direction.
 *
 * Represents a light source at infinite distance (like the sun) that affects
 * all objects equally regardless of their position. Only has direction and
 * intensity - no position or attenuation.
 * 
 * Commonly used for: sunlight, moonlight, global ambient lighting.
 */
struct DirectionalLight {
  glm::vec3 direction; ///< Light direction vector (normalized)
  
  // Phong lighting model color components
  glm::vec4 ambient;  ///< Ambient light color and intensity
  glm::vec4 diffuse;  ///< Diffuse light color and intensity
  glm::vec4 specular; ///< Specular light color and intensity

  /**
   * @brief Send light parameters to shader uniforms.
   * @param shader Target shader program with directionalLight uniforms
   */
  void Render(Shader &shader);
};

/**
 * @struct SpotLight
 * @brief Cone-shaped light source with position, direction, and angle.
 *
 * Represents a spotlight that emits light in a cone from a position toward
 * a direction. Has inner and outer cone angles for smooth falloff at edges.
 * Also includes distance attenuation like point lights.
 * 
 * Commonly used for: flashlights, car headlights, stage spotlights, street lamps.
 */
struct SpotLight {
  glm::vec3 position;  ///< World space position of the light source
  glm::vec3 direction; ///< Direction the spotlight is pointing (normalized)

  float cutOff;      ///< Inner cone angle (cosine value for efficiency)
  float outerCutOff; ///< Outer cone angle for smooth edge falloff (cosine value)

  // Attenuation coefficients for distance-based falloff
  float k0; ///< Constant attenuation factor (usually 1.0)
  float k1; ///< Linear attenuation factor
  float k2; ///< Quadratic attenuation factor

  // Phong lighting model color components
  glm::vec4 ambient;  ///< Ambient light color and intensity
  glm::vec4 diffuse;  ///< Diffuse light color and intensity
  glm::vec4 specular; ///< Specular light color and intensity

  /**
   * @brief Send light parameters to shader as uniform array element.
   * @param shader Target shader program with spotLights uniform array
   * @param idx Index in the spotLights array (0-based)
   */
  void Render(Shader &shader, int idx);
};

#endif
