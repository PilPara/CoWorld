/**
 * @file assimpGLMHelpers.hpp
 * @brief Utility functions for converting between Assimp and GLM data types
 */

#ifndef ASSIMP_GLM_HELPERS_H
#define ASSIMP_GLM_HELPERS_H

#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * @class AssimpGLMHelpers
 * @brief Static utility class for Assimp to GLM conversions
 *
 * Provides conversion functions between Assimp's data structures
 * and GLM's mathematical types. All methods are static inline for
 * header-only usage and optimal performance.
 */
class AssimpGLMHelpers {
public:
  /**
   * @brief Convert Assimp 4x4 matrix to GLM format
   *
   * Assimp uses row-major matrices while GLM uses column-major.
   * This function transposes and converts the matrix format.
   *
   * @param from Assimp matrix (row-major)
   * @return GLM matrix (column-major)
   */
  static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4 &from) {
    glm::mat4 to;
    // Assimp: a,b,c,d = row; 1,2,3,4 = column
    // GLM: [column][row] indexing
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
  }

  /**
   * @brief Convert Assimp 3D vector to GLM format
   *
   * Direct component-wise conversion.
   *
   * @param vec Assimp vector3
   * @return GLM vec3
   */
  static inline glm::vec3 GetGLMVec(const aiVector3D &vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
  }

  /**
   * @brief Convert Assimp quaternion to GLM format
   *
   * Note: Assimp and GLM use different quaternion component ordering.
   * Assimp: (x, y, z, w)
   * GLM: (w, x, y, z)
   *
   * @param pOrientation Assimp quaternion
   * @return GLM quaternion with corrected component order
   */
  static inline glm::quat GetGLMQuat(const aiQuaternion &pOrientation) {
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y,
                     pOrientation.z);
  }
};

#endif // !ASSIMP_GLM_HELPERS_H
