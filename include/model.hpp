/**
 * @file model.hpp
 * @brief Declaration of the Model class and collision management system.
 * 
 * This file defines the Model class for loading and rendering 3D models using Assimp,
 * along with collision detection and management utilities.
 */

#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include "mesh.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include <cfloat>
#include <memory>

/**
 * @brief Axis-aligned bounding box for collision detection.
 * 
 * Contains minimum and maximum coordinates, center point, and radius
 * for efficient collision detection calculations.
 */
struct BoundingBox {
  glm::vec3 min, max;  ///< Minimum and maximum coordinates
  glm::vec3 center;    ///< Center point of the bounding box
  float radius;        ///< Radius for sphere-based collision detection
};

/**
 * @brief 3D Model class for loading and rendering static models.
 * 
 * This class handles loading 3D models using Assimp, managing their
 * meshes and textures, and providing collision detection capabilities.
 * Models can be positioned, scaled, and rotated in 3D space.
 */
class Model {
public:
  glm::vec3 pos;                        ///< World position of the model
  glm::vec3 size;                       ///< Scale factor for each axis
  float rotation = 0.0f;                ///< Y-axis rotation in radians
  glm::vec3 additionalRotation = glm::vec3(0.0f); ///< Additional rotation on each axis
  BoundingBox boundingBox;              ///< Collision bounding box

  /**
   * @brief Constructor for Model.
   * @param pos Initial world position (default: origin)
   * @param size Scale factor for each axis (default: unit scale)
   * @param noTex Whether to load textures or use material colors only
   */
  Model(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f),
        bool noTex = false);

  /**
   * @brief Virtual destructor to ensure proper cleanup.
   */
  virtual ~Model();

  /**
   * @brief Rotate the model by specified degrees around an axis.
   * @param degrees Rotation amount in degrees
   * @param axis Rotation axis (x, y, or z components)
   */
  void Rotate(float degrees, glm::vec3 axis) {
    float radians = glm::radians(degrees);
    if (axis.x != 0.0f)
      additionalRotation.x = radians;
    if (axis.y != 0.0f)
      additionalRotation.y = radians;
    if (axis.z != 0.0f)
      additionalRotation.z = radians;
  }

  /**
   * @brief Initialize the model (currently empty implementation).
   */
  void Init();

  /**
   * @brief Load a 3D model from file using Assimp.
   * @param path File path to the model to load
   */
  void LoadModel(std::string path);

  /**
   * @brief Render the model using the specified shader.
   * @param shader Shader program to use for rendering
   */
  void Render(Shader &shader);

  /**
   * @brief Clean up GPU resources used by the model.
   */
  void Cleanup();

  /**
   * @brief Calculate the axis-aligned bounding box for collision detection.
   * 
   * Updates the boundingBox member with min/max coordinates, center,
   * and radius based on the model's current transform.
   */
  virtual void CalculateBoundingBox();

  /**
   * @brief Check if this model collides with another model.
   * @param other The other model to check collision against
   * @return True if the models' bounding boxes overlap
   */
  bool CollidesWith(const Model &other) const;

protected:
  bool noTex;                           ///< Flag indicating if textures should be loaded
  std::vector<Mesh> meshes;             ///< Collection of meshes that make up this model
  std::string directory;                ///< Directory path where the model file is located

  /**
   * @brief Process a node in the Assimp scene tree recursively.
   * @param node Current node to process
   * @param scene The Assimp scene containing all model data
   */
  void ProcessNode(aiNode *node, const aiScene *scene);

  /**
   * @brief Process a single mesh from the Assimp scene.
   * @param mesh The Assimp mesh to process
   * @param scene The Assimp scene containing material data
   * @return Processed Mesh object ready for rendering
   */
  Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

  /**
   * @brief Load textures of a specific type from a material.
   * @param mat The Assimp material to extract textures from
   * @param type The type of texture to load (diffuse, specular, etc.)
   * @return Vector of loaded Texture objects
   */
  std::vector<Texture> loadTextures(aiMaterial *mat, aiTextureType type);
};

/**
 * @brief Collision detection and management utilities.
 * 
 * Provides static methods for checking collisions between different entities
 * in the scene, including animated models and camera movements.
 */
class CollisionManager {
public:
  /**
   * @brief Check if a cow's movement to a new position causes collisions.
   * @param cow Pointer to the animated cow model
   * @param newPos The proposed new position for the cow
   * @param staticModels Vector of static models to check against
   * @return True if movement is allowed, false if collision detected
   */
  static bool
  CheckCowMovement(class AnimatedModel *cow, const glm::vec3 &newPos,
                   const std::vector<std::unique_ptr<Model>> &staticModels);
  
  /**
   * @brief Check if camera movement to a new position causes collisions.
   * @param newPos The proposed new camera position
   * @param cow Pointer to the animated cow model to maintain distance from
   * @param staticModels Vector of static models to check against
   * @return True if movement is allowed, false if collision detected
   */
  static bool
  CheckCameraMovement(const glm::vec3 &newPos, const class AnimatedModel *cow,
                      const std::vector<std::unique_ptr<Model>> &staticModels);
};

#endif
