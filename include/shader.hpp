/**
 * @file shader.hpp
 * @brief Declaration of the Shader class for OpenGL shader program management.
 * 
 * This file defines the Shader class which handles loading, compiling, linking,
 * and using OpenGL shader programs. It provides convenient methods for setting
 * uniform variables and managing shader lifecycle with RAII principles.
 */

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <assimp/color4.h>

/**
 * @brief OpenGL shader program wrapper with RAII management.
 * 
 * The Shader class provides a high-level interface for working with OpenGL
 * shader programs. It handles loading shader source files, compiling vertex
 * and fragment shaders, linking them into a program, and provides convenient
 * methods for setting uniform variables. The class follows RAII principles
 * and supports move semantics but prohibits copying.
 */
class Shader {
public:
  GLuint id;  ///< OpenGL shader program ID

  /**
   * @brief Default constructor that creates an uninitialized shader.
   * 
   * Creates a shader object with ID 0. Use Generate() or the parameterized
   * constructor to actually create a working shader program.
   */
  Shader();

  /**
   * @brief Constructor that loads and compiles a shader program from files.
   * @param vertexShaderPath Path to the vertex shader source file
   * @param fragmentShaderPath Path to the fragment shader source file
   * @throws std::runtime_error If shader compilation or linking fails
   * 
   * Automatically loads, compiles, and links the vertex and fragment shaders
   * into a complete shader program ready for use.
   */
  Shader(const char *vertexShaderPath, const char *fragmentShaderPath);

  /**
   * @brief Destructor that cleans up OpenGL resources.
   * 
   * Automatically deletes the shader program when the object is destroyed.
   */
  ~Shader();

  // =========================================================================
  // Copy semantics (deleted to prevent resource duplication)
  // =========================================================================

  /**
   * @brief Deleted copy constructor to prevent resource duplication.
   */
  Shader(const Shader &) = delete;

  /**
   * @brief Deleted copy assignment to prevent resource duplication.
   */
  Shader &operator=(const Shader &) = delete;

  // =========================================================================
  // Move semantics (for efficient resource transfer)
  // =========================================================================

  /**
   * @brief Move constructor for efficient resource transfer.
   * @param other Shader object to move from
   */
  Shader(Shader &&other) noexcept;

  /**
   * @brief Move assignment operator for efficient resource transfer.
   * @param other Shader object to move from
   * @return Reference to this shader
   */
  Shader &operator=(Shader &&other) noexcept;

  // =========================================================================
  // Shader Management
  // =========================================================================

  /**
   * @brief Load, compile, and link a shader program from source files.
   * @param vertexShaderPath Path to the vertex shader source file
   * @param fragmentShaderPath Path to the fragment shader source file
   * @return True if successful, false if compilation or linking failed
   * 
   * This method can be used to initialize a default-constructed shader
   * or to replace an existing shader program.
   */
  bool Generate(const char *vertexShaderPath, const char *fragmentShaderPath);

  /**
   * @brief Activate this shader program for rendering.
   * 
   * Makes this shader program the active one in the OpenGL context.
   * All subsequent draw calls will use this shader until another is activated.
   */
  void Activate();

  // =========================================================================
  // Uniform Variable Setters
  // =========================================================================

  /**
   * @brief Set a boolean uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param val Boolean value to set
   */
  void SetBool(const std::string &name, bool val);

  /**
   * @brief Set an integer uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param val Integer value to set
   */
  void SetInt(const std::string &name, int val);

  /**
   * @brief Set a float uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param val Float value to set
   */
  void SetFloat(const std::string &name, float val);

  /**
   * @brief Set a 3-component float vector uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param v GLM vec3 containing the values
   */
  void Set3Float(const std::string &name, const glm::vec3 &v);

  /**
   * @brief Set a 3-component float vector uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param v1 First component (x)
   * @param v2 Second component (y)
   * @param v3 Third component (z)
   */
  void Set3Float(const std::string &name, float v1, float v2, float v3);

  /**
   * @brief Set a 4-component float vector uniform from Assimp color.
   * @param name Name of the uniform variable in the shader
   * @param color Assimp color structure containing RGBA values
   */
  void Set4Float(const std::string &name, const aiColor4D &color);

  /**
   * @brief Set a 4-component float vector uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param v GLM vec4 containing the values
   */
  void Set4Float(const std::string &name, const glm::vec4 &v);

  /**
   * @brief Set a 4-component float vector uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param v1 First component (x)
   * @param v2 Second component (y)
   * @param v3 Third component (z)
   * @param v4 Fourth component (w)
   */
  void Set4Float(const std::string &name, float v1, float v2, float v3,
                 float v4);

  /**
   * @brief Set a 4x4 matrix uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param val GLM mat4 matrix to set
   */
  void SetMat4(const std::string &name, const glm::mat4 &val);

private:
  // =========================================================================
  // Internal Helper Methods
  // =========================================================================

  /**
   * @brief Load shader source code from a file.
   * @param filePath Path to the shader source file
   * @return String containing the shader source code, empty if file not found
   * 
   * Reads the entire contents of a shader source file into a string.
   * Prints an error message if the file cannot be opened.
   */
  std::string LoadShaderSource(const char *filePath);

  /**
   * @brief Compile a shader from source file.
   * @param filePath Path to the shader source file
   * @param type OpenGL shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
   * @return Compiled shader ID, or 0 if compilation failed
   * 
   * Loads, compiles, and validates a shader. Prints detailed error messages
   * if compilation fails. The caller is responsible for deleting the shader.
   */
  GLuint CompileShader(const char *filePath, GLenum type);

  /**
   * @brief Clean up OpenGL shader program resources.
   * 
   * Deletes the OpenGL shader program and resets the ID to 0.
   * Safe to call multiple times.
   */
  void Cleanup();
};

#endif // !SHADER_H
