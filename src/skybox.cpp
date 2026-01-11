/**
 * @file skybox.cpp
 * @brief Implementation of the Skybox class for environment background rendering.
 * 
 * Contains the complete implementation of the Skybox class, including cubemap
 * texture loading, cube geometry setup, and rendering with special depth testing.
 * The skybox creates a 360-degree background environment that appears at infinite
 * distance from the viewer.
 */

#include "skybox.hpp"
#include <stb/stb_image.h>
#include <iostream>

/**
 * @brief Static vertex data for a unit cube used in skybox rendering.
 * 
 * Contains 36 vertices (12 triangles) that form a complete cube.
 * Each vertex has 3 position coordinates (x, y, z). The cube is centered
 * at the origin and extends from -1 to +1 on each axis.
 */
static float skyboxVertices[] = {
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
    -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,
    -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

/**
 * @brief Constructor that creates a skybox from six cubemap face images.
 * @param faces Vector of six file paths for cubemap faces in order:
 *              [right, left, top, bottom, front, back]
 * 
 * Initializes the skybox by:
 * 1. Loading and compiling the skybox shader
 * 2. Creating VAO and VBO for cube geometry
 * 3. Setting up vertex attributes for position data
 * 4. Loading the cubemap texture from the provided face images
 */
Skybox::Skybox(const std::vector<std::string> &faces)
    : shader("shaders/skybox.vs", "shaders/skybox.fs") {
  // Setup cube VAO
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  cubemapTexture = LoadCubemap(faces);
}

/**
 * @brief Destructor that cleans up OpenGL resources.
 * 
 * Deletes the vertex array object and vertex buffer object to prevent
 * resource leaks. The cubemap texture and shader are cleaned up automatically
 * by their respective destructors.
 */
Skybox::~Skybox() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

/**
 * @brief Render the skybox using the provided view and projection matrices.
 * @param view View matrix (translation will be removed)
 * @param projection Projection matrix for proper perspective
 * 
 * Renders the skybox with special handling to ensure it appears behind all geometry:
 * 1. Changes depth function to GL_LEQUAL to allow skybox at maximum depth
 * 2. Removes translation from view matrix to simulate infinite distance
 * 3. Binds and renders the cube geometry with cubemap texture
 * 4. Restores normal depth function for subsequent rendering
 */
void Skybox::Render(const glm::mat4 &view, const glm::mat4 &projection) {
  glDepthFunc(GL_LEQUAL);
  shader.Activate();
  glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
  shader.SetMat4("view", viewNoTranslate);
  shader.SetMat4("projection", projection);

  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
}

/**
 * @brief Load six images into a cubemap texture.
 * @param faces Vector of six file paths for cubemap faces
 * @return OpenGL texture ID of the created cubemap
 * 
 * Creates a cubemap texture by loading six individual images for each face:
 * - faces[0] = Right (+X)
 * - faces[1] = Left (-X)  
 * - faces[2] = Top (+Y)
 * - faces[3] = Bottom (-Y)
 * - faces[4] = Front (+Z)
 * - faces[5] = Back (-Z)
 * 
 * Sets appropriate texture parameters for seamless cubemap sampling and
 * handles loading errors with informative error messages.
 */
unsigned int Skybox::LoadCubemap(const std::vector<std::string> &faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(false);
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "[ERROR::SKYBOX] Failed to load: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}
