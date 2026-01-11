/**
 * @file model.cpp
 * @brief Implementation of the Model class and collision handling.
 * 
 * Contains the implementation of the Model class for loading and rendering 3D models,
 * along with the CollisionManager class for handling collision detection between
 * different game entities.
 */

#include "model.hpp"
#include "animated_model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
 * @brief Constructs a Model with the specified position, size, and texture settings.
 * @param pos Initial world position of the model
 * @param size Scale factor for each axis
 * @param noTex Whether to load textures or use material colors only
 */
Model::Model(glm::vec3 pos, glm::vec3 size, bool noTex) {
  this->pos = pos;
  this->size = size;
  this->noTex = noTex;
}

/**
 * @brief Initializes the model (currently empty implementation).
 * 
 * This method is reserved for future initialization logic that may be
 * needed after model construction.
 */
void Model::Init() {}

/**
 * @brief Destructor that ensures proper cleanup of resources.
 */
Model::~Model() { Cleanup(); }

/**
 * @brief Loads a 3D model from file using the Assimp library.
 * @param path File path to the model to load
 * 
 * This method uses Assimp to load the model file, processing all nodes
 * and meshes recursively. After loading, it calculates the bounding box
 * for collision detection. If loading fails, an error message is printed.
 */
void Model::LoadModel(std::string path) {
  Assimp::Importer import;
  const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "[ERROR::MODEL::LOADING] Failed to load model at " << path
              << ": " << import.GetErrorString() << std::endl;
    return;
  }

  this->directory = path.substr(0, path.find_last_of("/"));
  ProcessNode(scene->mRootNode, scene);
  CalculateBoundingBox();
}

/**
 * @brief Renders the model using the specified shader.
 * @param shader Shader program to use for rendering
 * 
 * Constructs the model transformation matrix by applying translation,
 * rotations, and scaling in the correct order. Sets the transformation
 * matrix and material properties in the shader, then renders all meshes.
 */
void Model::Render(Shader &shader) {
  glm::mat4 modelMat = glm::mat4(1.0f);
  modelMat = glm::translate(modelMat, this->pos);

  if (additionalRotation.x != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.x, glm::vec3(1, 0, 0));
  if (additionalRotation.y != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.y, glm::vec3(0, 1, 0));
  if (additionalRotation.z != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.z, glm::vec3(0, 0, 1));

  modelMat = glm::rotate(modelMat, this->rotation, glm::vec3(0, 1, 0));
  modelMat = glm::scale(modelMat, this->size);

  shader.SetMat4("model", modelMat);
  shader.SetFloat("material.shininess", 0.5f);

  for (Mesh &mesh : this->meshes) {
    mesh.Render(shader);
  }
}

/**
 * @brief Cleans up GPU resources used by the model.
 * 
 * Calls cleanup on all meshes to release vertex buffers, textures,
 * and other GPU resources. Should be called before the model is destroyed.
 */
void Model::Cleanup() {
  for (Mesh &mesh : this->meshes) {
    mesh.Cleanup();
  }
}

/**
 * @brief Processes a node in the Assimp scene tree recursively.
 * @param node Current node to process
 * @param scene The Assimp scene containing all model data
 * 
 * Processes all meshes attached to the current node, then recursively
 * processes all child nodes to build the complete model mesh collection.
 */
void Model::ProcessNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    this->meshes.push_back(ProcessMesh(mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

/**
 * @brief Processes a single mesh from the Assimp scene.
 * @param mesh The Assimp mesh to process
 * @param scene The Assimp scene containing material data
 * @return Processed Mesh object ready for rendering
 * 
 * Extracts vertex attributes (position, normal, texture coordinates),
 * face indices, and material/texture information from the Assimp mesh.
 * Handles both textured and non-textured materials based on the noTex flag.
 */
Mesh Model::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Extract vertex attributes
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                           mesh->mVertices[i].z);
    vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                              mesh->mNormals[i].z);

    if (mesh->mTextureCoords[0]) {
      vertex.texCoord =
          glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    } else {
      vertex.texCoord = glm::vec2(0.0f);
    }

    vertices.push_back(vertex);
  }

  // Extract indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Load materials
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    if (this->noTex) {
      aiColor4D diff(1.0f);
      aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diff);

      aiColor4D spec(1.0f);
      aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &spec);

      return Mesh(vertices, indices, diff, spec);
    }

    auto loadTexType = [&](aiTextureType type) {
      std::vector<Texture> texs;
      for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        aiString str;
        material->GetTexture(type, i, &str);
        std::string filename = std::string(str.C_Str());

        Texture tex(this->directory, filename, type);
        tex.Load(false);
        texs.push_back(std::move(tex));
      }
      return texs;
    };

    auto diffuseMaps = loadTexType(aiTextureType_DIFFUSE);
    textures.insert(textures.end(),
                    std::make_move_iterator(diffuseMaps.begin()),
                    std::make_move_iterator(diffuseMaps.end()));

    auto specularMaps = loadTexType(aiTextureType_SPECULAR);
    textures.insert(textures.end(),
                    std::make_move_iterator(specularMaps.begin()),
                    std::make_move_iterator(specularMaps.end()));
  }

  return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

/**
 * @brief Loads textures of a specific type from a material.
 * @param mat The Assimp material to extract textures from
 * @param type The type of texture to load (diffuse, specular, etc.)
 * @return Vector of loaded Texture objects
 * 
 * Iterates through all textures of the specified type in the material,
 * loads each texture from disk, and returns them as a vector of Texture objects.
 */
std::vector<Texture> Model::loadTextures(aiMaterial *mat, aiTextureType type) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    std::string filename = std::string(str.C_Str());

    Texture tex(this->directory, filename, type);
    tex.Load(false);
    textures.push_back(std::move(tex));
  }
  return textures;
}

/**
 * @brief Calculates the axis-aligned bounding box for collision detection.
 * 
 * Transforms all vertices of all meshes by the current model transformation matrix,
 * then finds the minimum and maximum coordinates to define the bounding box.
 * Also calculates the center point and radius for efficient collision detection.
 */
void Model::CalculateBoundingBox() {
  if (meshes.empty())
    return;

  glm::mat4 modelMat = glm::mat4(1.0f);
  modelMat = glm::translate(modelMat, this->pos);

  if (additionalRotation.x != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.x, glm::vec3(1, 0, 0));
  if (additionalRotation.y != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.y, glm::vec3(0, 1, 0));
  if (additionalRotation.z != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.z, glm::vec3(0, 0, 1));

  modelMat = glm::rotate(modelMat, this->rotation, glm::vec3(0, 1, 0));
  modelMat = glm::scale(modelMat, this->size);

  glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);

  for (const auto &mesh : meshes) {
    for (const auto &vertex : mesh.vertices) {
      glm::vec4 worldPos = modelMat * glm::vec4(vertex.pos, 1.0f);
      glm::vec3 v = glm::vec3(worldPos);
      minPos = glm::min(minPos, v);
      maxPos = glm::max(maxPos, v);
    }
  }

  boundingBox.min = minPos;
  boundingBox.max = maxPos;
  boundingBox.center = (minPos + maxPos) * 0.5f;

  glm::vec3 extent = maxPos - minPos;
  boundingBox.radius = glm::max(extent.x, extent.z) * 0.5f;
}

/**
 * @brief Checks if this model collides with another model.
 * @param other The other model to check collision against
 * @return True if the models' bounding boxes overlap, false otherwise
 * 
 * Performs axis-aligned bounding box (AABB) collision detection by checking
 * for overlap on all three axes (X, Y, Z). Returns true if there is overlap
 * on all axes, indicating a collision.
 */
bool Model::CollidesWith(const Model &other) const {
  return (boundingBox.min.x <= other.boundingBox.max.x &&
          boundingBox.max.x >= other.boundingBox.min.x) &&
         (boundingBox.min.y <= other.boundingBox.max.y &&
          boundingBox.max.y >= other.boundingBox.min.y) &&
         (boundingBox.min.z <= other.boundingBox.max.z &&
          boundingBox.max.z >= other.boundingBox.min.z);
}

// ===============================================================================
// CollisionManager Implementation
// ===============================================================================

/**
 * @brief Checks if a cow's movement to a new position causes collisions.
 * @param cow Pointer to the animated cow model
 * @param newPos The proposed new position for the cow
 * @param staticModels Vector of static models to check against
 * @return True if movement is allowed, false if collision detected
 * 
 * Temporarily moves the cow to the new position, updates its bounding box,
 * then checks for collisions with all static models (except the ground plane).
 * Uses collision margins defined in the Config class. If a collision is detected,
 * the cow is moved back to its original position.
 */
bool CollisionManager::CheckCowMovement(
    AnimatedModel *cow, const glm::vec3 &newPos,
    const std::vector<std::unique_ptr<Model>> &staticModels) {
  glm::vec3 oldPos = cow->pos;
  cow->pos = newPos;
  cow->CalculateBoundingBox();

  const BoundingBox &cowBox = cow->boundingBox;

  for (size_t i = 1; i < staticModels.size(); i++) { // skip ground plane
    staticModels[i]->CalculateBoundingBox();
    const BoundingBox &staticBox = staticModels[i]->boundingBox;

    if (cowBox.max.y >
        staticBox.min.y + Config::Collision::COW_BUILDING_MARGIN) {
      bool xOverlap =
          (cowBox.min.x - Config::Collision::COW_COLLISION_MARGIN) <=
              staticBox.max.x &&
          (cowBox.max.x + Config::Collision::COW_COLLISION_MARGIN) >=
              staticBox.min.x;
      bool zOverlap =
          (cowBox.min.z - Config::Collision::COW_COLLISION_MARGIN) <=
              staticBox.max.z &&
          (cowBox.max.z + Config::Collision::COW_COLLISION_MARGIN) >=
              staticBox.min.z;

      if (xOverlap && zOverlap) {
        cow->pos = oldPos;
        cow->CalculateBoundingBox();
        return false;
      }
    }
  }
  return true;
}

/**
 * @brief Checks if camera movement to a new position causes collisions.
 * @param newPos The proposed new camera position
 * @param cow Pointer to the animated cow model to maintain distance from
 * @param staticModels Vector of static models to check against
 * @return True if movement is allowed, false if collision detected
 * 
 * Performs collision detection for camera movement by checking:
 * 1. Minimum distance from the cow (prevents camera from getting too close)
 * 2. Collision with static models using expanded bounding boxes with margins
 * 3. Minimum height constraint to prevent camera from going underground
 */
bool CollisionManager::CheckCameraMovement(
    const glm::vec3 &newPos, const AnimatedModel *cow,
    const std::vector<std::unique_ptr<Model>> &staticModels) {
  const float minCowDistance = Config::Camera::MIN_COW_DISTANCE;
  const float cameraRadius = Config::Camera::COLLISION_RADIUS;

  if (glm::length(newPos - cow->pos) < minCowDistance) {
    return false;
  }

  glm::vec3 cameraMin = newPos - glm::vec3(cameraRadius);
  glm::vec3 cameraMax = newPos + glm::vec3(cameraRadius);

  for (size_t i = 1; i < staticModels.size(); i++) { // skip plane
    const BoundingBox &objBox = staticModels[i]->boundingBox;

    glm::vec3 margin(Config::Collision::CAMERA_COLLISION_MARGIN);
    glm::vec3 expandedMin = objBox.min - margin;
    glm::vec3 expandedMax = objBox.max + margin;

    if ((cameraMin.x <= expandedMax.x && cameraMax.x >= expandedMin.x) &&
        (cameraMin.y <= expandedMax.y && cameraMax.y >= expandedMin.y) &&
        (cameraMin.z <= expandedMax.z && cameraMax.z >= expandedMin.z)) {
      return false;
    }
  }

  if (newPos.y < Config::Camera::MIN_HEIGHT) {
    return false;
  }
  return true;
}
