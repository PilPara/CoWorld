/**
 * @file shader.cpp
 * @brief Implementation of the Shader class for OpenGL shader management.
 * 
 * Contains the complete implementation of the Shader class, including shader
 * loading, compilation, linking, uniform variable setting, and resource management.
 * Provides robust error handling and follows RAII principles for automatic
 * resource cleanup.
 */

#include "shader.hpp"
#include "config.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief Default constructor that creates an uninitialized shader.
 * 
 * Initializes the shader ID to 0, indicating no shader program is loaded.
 */
Shader::Shader() : id(0) {}

/**
 * @brief Constructor that loads and compiles a shader program from files.
 * @param vertexShaderPath Path to the vertex shader source file
 * @param fragmentShaderPath Path to the fragment shader source file
 * @throws std::runtime_error If shader compilation or linking fails
 * 
 * Automatically loads, compiles, and links the vertex and fragment shaders
 * into a complete shader program. Throws an exception if any step fails.
 */
Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath)
    : id(0) {
  if (!Generate(vertexShaderPath, fragmentShaderPath)) {
    throw std::runtime_error(Config::Shader::ERR_RUNTIME);
  }
}

/**
 * @brief Destructor that cleans up OpenGL resources.
 * 
 * Automatically deletes the shader program when the object is destroyed,
 * ensuring no resource leaks occur.
 */
Shader::~Shader() { Cleanup(); }

/**
 * @brief Move constructor for efficient resource transfer.
 * @param other Shader object to move from
 * 
 * Transfers ownership of the OpenGL shader program from the other object,
 * leaving it in a valid but empty state.
 */
Shader::Shader(Shader &&other) noexcept : id(other.id) { other.id = 0; }

/**
 * @brief Move assignment operator for efficient resource transfer.
 * @param other Shader object to move from
 * @return Reference to this shader
 * 
 * Cleans up current resources, then transfers ownership from the other object.
 * Handles self-assignment safely.
 */
Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    Cleanup();
    this->id = other.id;
    other.id = 0;
  }
  return *this;
}

/**
 * @brief Load, compile, and link a shader program from source files.
 * @param vertexShaderPath Path to the vertex shader source file
 * @param fragmentShaderPath Path to the fragment shader source file
 * @return True if successful, false if compilation or linking failed
 * 
 * Performs the complete shader program creation process:
 * 1. Compiles vertex and fragment shaders from source files
 * 2. Creates a shader program and attaches the compiled shaders
 * 3. Links the program and validates the result
 * 4. Cleans up intermediate shader objects
 * 
 * Provides detailed error messages for each potential failure point.
 */
bool Shader::Generate(const char *vertexShaderPath,
                      const char *fragmentShaderPath) {
  GLuint vertexShader = CompileShader(vertexShaderPath, GL_VERTEX_SHADER);
  if (vertexShader == 0)
    return false;

  GLuint fragmentShader = CompileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
  if (fragmentShader == 0) {
    glDeleteShader(vertexShader);
    return false;
  }

  this->id = glCreateProgram();
  if (this->id == 0) {
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return false;
  }

  glAttachShader(this->id, vertexShader);
  glAttachShader(this->id, fragmentShader);
  glLinkProgram(this->id);

  GLint success;
  glGetProgramiv(this->id, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[Config::Shader::LOG_BUF_SIZE];
    glGetProgramInfoLog(this->id, Config::Shader::LOG_BUF_SIZE, NULL, infoLog);
    std::cout << Config::Shader::ERR_LINK << " " << infoLog << std::endl;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(this->id);
    this->id = 0;
    return false;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return true;
}

/**
 * @brief Activate this shader program for rendering.
 * 
 * Makes this shader program the active one in the OpenGL context.
 * All subsequent draw calls will use this shader until another is activated.
 */
void Shader::Activate() { glUseProgram(this->id); }

// ===============================================================================
// Uniform Variable Setters
// ===============================================================================

/**
 * @brief Set a boolean uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param val Boolean value to set
 * 
 * Converts the boolean to an integer (0 or 1) as required by OpenGL.
 */
void Shader::SetBool(const std::string &name, bool val) {
  glUniform1i(glGetUniformLocation(this->id, name.c_str()), val);
}

/**
 * @brief Set an integer uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param val Integer value to set
 */
void Shader::SetInt(const std::string &name, int val) {
  glUniform1i(glGetUniformLocation(this->id, name.c_str()), val);
}

/**
 * @brief Set a float uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param val Float value to set
 */
void Shader::SetFloat(const std::string &name, float val) {
  glUniform1f(glGetUniformLocation(this->id, name.c_str()), val);
}

/**
 * @brief Set a 3-component float vector uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param v GLM vec3 containing the values
 * 
 * Convenience method that extracts components from a GLM vector.
 */
void Shader::Set3Float(const std::string &name, const glm::vec3 &v) {
  Set3Float(name, v.x, v.y, v.z);
}

/**
 * @brief Set a 3-component float vector uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param v1 First component (x)
 * @param v2 Second component (y)
 * @param v3 Third component (z)
 */
void Shader::Set3Float(const std::string &name, float v1, float v2, float v3) {
  glUniform3f(glGetUniformLocation(this->id, name.c_str()), v1, v2, v3);
}

/**
 * @brief Set a 4-component float vector uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param v1 First component (x)
 * @param v2 Second component (y)  
 * @param v3 Third component (z)
 * @param v4 Fourth component (w)
 */
void Shader::Set4Float(const std::string &name, float v1, float v2, float v3,
                       float v4) {
  glUniform4f(glGetUniformLocation(this->id, name.c_str()), v1, v2, v3, v4);
}

/**
 * @brief Set a 4-component float vector uniform from Assimp color.
 * @param name Name of the uniform variable in the shader
 * @param color Assimp color structure containing RGBA values
 * 
 * Convenience method for setting colors from Assimp material properties.
 */
void Shader::Set4Float(const std::string &name, const aiColor4D &color) {
  glUniform4f(glGetUniformLocation(this->id, name.c_str()), color.r, color.g,
              color.b, color.a);
}

/**
 * @brief Set a 4-component float vector uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param v GLM vec4 containing the values
 * 
 * Convenience method that extracts components from a GLM vector.
 */
void Shader::Set4Float(const std::string &name, const glm::vec4 &v) {
  glUniform4f(glGetUniformLocation(this->id, name.c_str()), v.x, v.y, v.z, v.w);
}

/**
 * @brief Set a 4x4 matrix uniform variable.
 * @param name Name of the uniform variable in the shader
 * @param val GLM mat4 matrix to set
 * 
 * Uploads a 4x4 transformation matrix to the shader. Commonly used
 * for model, view, projection, and combined transformation matrices.
 */
void Shader::SetMat4(const std::string &name, const glm::mat4 &val) {
  glUniformMatrix4fv(glGetUniformLocation(this->id, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(val));
}

// ===============================================================================
// Internal Helper Methods
// ===============================================================================

/**
 * @brief Load shader source code from a file.
 * @param filePath Path to the shader source file
 * @return String containing the shader source code, empty if file not found
 * 
 * Reads the entire contents of a shader source file into a string.
 * Handles file I/O errors gracefully and prints error messages.
 */
std::string Shader::LoadShaderSource(const char *filePath) {
  std::ifstream file;
  std::stringstream buf;
  std::string contents;

  file.open(filePath);
  if (file.is_open()) {
    buf << file.rdbuf();
    contents = buf.str();
  } else {
    std::cout << Config::Shader::ERR_FILE << filePath << std::endl;
  }
  file.close();

  return contents;
}

/**
 * @brief Compile a shader from source file.
 * @param filePath Path to the shader source file
 * @param type OpenGL shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
 * @return Compiled shader ID, or 0 if compilation failed
 * 
 * Performs the complete shader compilation process:
 * 1. Loads shader source from file
 * 2. Creates a shader object of the specified type
 * 3. Compiles the shader and checks for errors
 * 4. Returns the compiled shader ID or 0 on failure
 * 
 * Provides detailed error messages including shader type and file path.
 */
GLuint Shader::CompileShader(const char *filePath, GLenum type) {
  std::string shaderSourceString = LoadShaderSource(filePath);
  if (shaderSourceString.empty()) {
    return 0;
  }

  GLuint shaderID = glCreateShader(type);
  if (shaderID == 0) {
    std::cout << Config::Shader::ERR_CREATE << std::endl;
    return 0;
  }

  const GLchar *shaderSource = shaderSourceString.c_str();
  glShaderSource(shaderID, 1, &shaderSource, NULL);
  glCompileShader(shaderID);

  GLint success;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[Config::Shader::LOG_BUF_SIZE];
    glGetShaderInfoLog(shaderID, Config::Shader::LOG_BUF_SIZE, NULL, infoLog);
    const char *shaderType = (type == GL_VERTEX_SHADER)
                                 ? Config::Shader::ERR_VERTEX
                                 : Config::Shader::ERR_FRAGMENT;
    std::cout << shaderType << " " << filePath << ": " << infoLog << std::endl;
    glDeleteShader(shaderID);
    return 0;
  }

  return shaderID;
}

/**
 * @brief Clean up OpenGL shader program resources.
 * 
 * Deletes the OpenGL shader program if it exists and resets the ID to 0.
 * Safe to call multiple times and handles the case where no program exists.
 */
void Shader::Cleanup() {
  if (this->id != 0) {
    glDeleteProgram(this->id);
    this->id = 0;
  }
}
