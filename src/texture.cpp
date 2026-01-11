/**
 * @file texture.cpp
 * @brief Implementation of the Texture class for OpenGL texture management.
 * 
 * This file implements the Texture class methods for loading images using
 * STB library, creating OpenGL texture objects, and managing texture resources
 * with RAII principles. It provides automatic color format detection and
 * proper OpenGL parameter configuration for 2D textures.
 */

#include "texture.hpp"
#include "config.hpp"
#include <stb/stb_image.h>
#include <iostream>

/**
 * @brief Default constructor that creates an uninitialized texture.
 * 
 * Initializes all members to default values: ID to 0, type to NONE,
 * and empty strings for directory and path.
 */
Texture::Texture() : id(0), type(aiTextureType_NONE) {}

/**
 * @brief Constructor that creates and generates a texture from file path.
 * @param dir Directory path containing the texture file
 * @param path Filename of the texture
 * @param type Assimp texture type classification
 * 
 * Stores the texture information and automatically generates an OpenGL
 * texture object. The actual image data loading is deferred until Load() is called.
 */
Texture::Texture(std::string dir, std::string path, aiTextureType type)
    : id(0), dir(std::move(dir)), path(std::move(path)), type(type) {
  Generate();
}

/**
 * @brief Destructor that automatically cleans up OpenGL texture resources.
 * 
 * Ensures proper cleanup of OpenGL resources when the texture object
 * is destroyed, preventing memory leaks.
 */
Texture::~Texture() { Cleanup(); }

/**
 * @brief Move constructor for efficient resource transfer.
 * @param other Texture object to move from
 * 
 * Transfers all resources from the source object, including the OpenGL
 * texture ID and metadata. The source object is left in a valid but
 * empty state with ID 0.
 */
Texture::Texture(Texture &&other) noexcept
    : id(other.id), type(other.type), dir(std::move(other.dir)),
      path(std::move(other.path)) {
  other.id = 0;
}

/**
 * @brief Move assignment operator for efficient resource transfer.
 * @param other Texture object to move from
 * @return Reference to this texture
 * 
 * Cleans up current resources before transferring ownership from the
 * source object. This ensures no resource leaks occur during assignment.
 */
Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    Cleanup();

    this->id = other.id;
    this->type = other.type;
    this->dir = std::move(other.dir);
    this->path = std::move(other.path);

    other.id = 0;
  }
  return *this;
}

/**
 * @brief Generate an OpenGL texture object if one doesn't exist.
 * 
 * Creates a new OpenGL 2D texture object using glGenTextures.
 * Safe to call multiple times - only generates if ID is currently 0.
 */
void Texture::Generate() {
  if (this->id == 0) {
    glGenTextures(1, &this->id);
  }
}

/**
 * @brief Load image data from file into the OpenGL texture.
 * @param flip Whether to flip the image vertically (default: true)
 * 
 * Uses STB image library to load the texture file and create an OpenGL
 * texture. Automatically detects the image format based on channel count:
 * - 1 channel: GL_RED (grayscale)
 * - 3 channels: GL_RGB (standard color)
 * - 4 channels: GL_RGBA (color with alpha)
 * 
 * Sets texture parameters from Config system including wrapping modes
 * and filtering options. Generates mipmaps for improved rendering quality
 * at different distances. Prints error messages if file loading fails.
 */
void Texture::Load(bool flip) {
  if (this->id == 0) {
    Generate();
  }

  stbi_set_flip_vertically_on_load(flip);

  int imageWidth = 0;
  int imageHeight = 0;
  int nChannels = 0;

  unsigned char *data = stbi_load((this->dir + "/" + this->path).c_str(),
                                  &imageWidth, &imageHeight, &nChannels, 0);

  GLenum colorMode = GL_RGB;
  switch (nChannels) {
  case 1:
    colorMode = GL_RED;
    break;
  case 4:
    colorMode = GL_RGBA;
    break;
  }

  if (data) {
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexImage2D(GL_TEXTURE_2D, 0, colorMode, imageWidth, imageHeight, 0,
                 colorMode, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    Config::Texture::DEFAULT_WRAP_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    Config::Texture::DEFAULT_WRAP_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    Config::Texture::DEFAULT_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    Config::Texture::DEFAULT_MIN_FILTER);
  } else {
    std::cout << Config::Texture::ERR_LOAD << this->dir << "/" << this->path
              << " (" << stbi_failure_reason() << ")" << std::endl;
  }

  stbi_image_free(data);
}

/**
 * @brief Bind this texture to the current OpenGL context.
 * 
 * Makes this texture active for subsequent rendering operations.
 * Only binds if the texture has been properly generated (ID != 0).
 * Must be called before drawing objects that use this texture.
 */
void Texture::Bind() {
  if (this->id != 0) {
    glBindTexture(GL_TEXTURE_2D, this->id);
  }
}

/**
 * @brief Clean up OpenGL texture resources.
 * 
 * Deletes the OpenGL texture object using glDeleteTextures and resets
 * the ID to 0. Safe to call multiple times and handles the case where
 * no texture was generated. Called automatically by destructor and
 * move assignment operator.
 */
void Texture::Cleanup() {
  if (this->id != 0) {
    glDeleteTextures(1, &this->id);
    this->id = 0;
  }
}
