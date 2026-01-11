/**
 * @file skybox.hpp
 * @brief Declaration of the Skybox class for rendering environment backgrounds.
 * 
 * This file defines the Skybox class which handles loading cubemap textures
 * and rendering a background environment. The skybox provides 360-degree
 * background imagery that creates the illusion of distant scenery.
 */

#ifndef SKYBOX_H
#define SKYBOX_H

#include "shader.hpp"
#include <glad/glad.h>
#include <vector>
#include <string>

/**
 * @brief Skybox class for rendering 360-degree background environments.
 * 
 * The Skybox class creates and manages a cubemap texture from six face images
 * and renders it as a large cube around the scene. The skybox is rendered
 * with the depth test modified to ensure it appears behind all other objects.
 * It uses a special shader that removes translation from the view matrix
 * to create the illusion of infinite distance.
 */
class Skybox {
public:
  /**
   * @brief Constructor that creates a skybox from six cubemap face images.
   * @param faces Vector of six file paths for cubemap faces in order:
   *              [right, left, top, bottom, front, back]
   * 
   * Loads the six texture faces and creates a cubemap texture. Also sets up
   * the cube geometry and initializes the skybox shader program.
   */
  Skybox(const std::vector<std::string> &faces);

  /**
   * @brief Destructor that cleans up OpenGL resources.
   * 
   * Deletes the vertex array object and vertex buffer object.
   * The cubemap texture and shader are cleaned up automatically.
   */
  ~Skybox();

  /**
   * @brief Render the skybox using the provided view and projection matrices.
   * @param view View matrix (translation will be removed)
   * @param projection Projection matrix for proper perspective
   * 
   * Renders the skybox cube with special depth testing to ensure it appears
   * behind all other geometry. The view matrix translation is removed to
   * create the illusion that the skybox is at infinite distance.
   */
  void Render(const glm::mat4 &view, const glm::mat4 &projection);

private:
  unsigned int cubemapTexture;  ///< OpenGL cubemap texture ID
  unsigned int VAO, VBO;        ///< Vertex array and buffer objects for cube geometry
  Shader shader;                ///< Shader program for skybox rendering

  /**
   * @brief Load six images into a cubemap texture.
   * @param faces Vector of six file paths for cubemap faces
   * @return OpenGL texture ID of the created cubemap
   * 
   * Loads each image using STB image loader and creates a cubemap texture
   * with appropriate filtering and wrapping parameters. Handles loading
   * errors gracefully with error messages.
   */
  unsigned int LoadCubemap(const std::vector<std::string> &faces);
};

#endif
