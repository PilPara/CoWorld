/**
 * @file texture.hpp
 * @brief Declaration of the Texture class for OpenGL texture management.
 * 
 * This file defines the Texture class which handles loading, generating, and
 * managing OpenGL 2D textures. It provides RAII-based resource management,
 * supports various image formats through STB library, and integrates with
 * Assimp material system for texture type classification.
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>
#include <assimp/material.h>

/**
 * @brief OpenGL texture wrapper with RAII management and move semantics.
 * 
 * The Texture class provides a high-level interface for working with OpenGL
 * 2D textures. It handles texture loading from image files using the STB
 * image library, manages OpenGL texture objects with RAII principles, and
 * supports various texture types as defined by Assimp. The class follows
 * move-only semantics to prevent resource duplication.
 */
class Texture {
public:
  unsigned int id;        ///< OpenGL texture object ID
  aiTextureType type;     ///< Assimp texture type (diffuse, specular, etc.)
  std::string dir;        ///< Directory path containing the texture file
  std::string path;       ///< Filename of the texture

  /**
   * @brief Default constructor that creates an uninitialized texture.
   * 
   * Creates a texture object with ID 0 and type NONE. Use the parameterized
   * constructor or call Generate() and Load() to create a working texture.
   */
  Texture();

  /**
   * @brief Constructor that creates and generates a texture from file path.
   * @param dir Directory path containing the texture file
   * @param path Filename of the texture
   * @param type Assimp texture type classification
   * 
   * Automatically generates an OpenGL texture object. Call Load() to
   * actually load the image data from disk.
   */
  Texture(std::string dir, std::string path, aiTextureType type);

  // =========================================================================
  // Rule of Five (Move-only semantics)
  // =========================================================================

  /**
   * @brief Destructor that cleans up OpenGL texture resources.
   * 
   * Automatically deletes the OpenGL texture object when destroyed.
   */
  ~Texture();

  /**
   * @brief Deleted copy constructor to prevent resource duplication.
   */
  Texture(const Texture &other) = delete;

  /**
   * @brief Deleted copy assignment to prevent resource duplication.
   */
  Texture &operator=(const Texture &other) = delete;

  /**
   * @brief Move constructor for efficient resource transfer.
   * @param other Texture object to move from
   * 
   * Transfers ownership of the OpenGL texture and associated data,
   * leaving the source object in a valid but empty state.
   */
  Texture(Texture &&other) noexcept;

  /**
   * @brief Move assignment operator for efficient resource transfer.
   * @param other Texture object to move from
   * @return Reference to this texture
   * 
   * Cleans up current resources, then transfers ownership from the other object.
   */
  Texture &operator=(Texture &&other) noexcept;

  // =========================================================================
  // Texture Management
  // =========================================================================

  /**
   * @brief Generate an OpenGL texture object.
   * 
   * Creates an OpenGL texture object if one doesn't already exist.
   * This must be called before loading texture data.
   */
  void Generate();

  /**
   * @brief Load image data from file into the OpenGL texture.
   * @param flip Whether to flip the image vertically (default: true)
   * 
   * Loads the image file specified by dir and path using STB image library.
   * Automatically determines the color format based on the number of channels
   * and sets appropriate texture parameters for filtering and wrapping.
   */
  void Load(bool flip = true);

  /**
   * @brief Bind this texture to the current OpenGL context.
   * 
   * Makes this texture active for subsequent rendering operations.
   * The texture must be loaded before binding.
   */
  void Bind();

private:
  /**
   * @brief Clean up OpenGL texture resources.
   * 
   * Deletes the OpenGL texture object and resets the ID to 0.
   * Safe to call multiple times.
   */
  void Cleanup();
};

#endif // ! TEXTURE_H
