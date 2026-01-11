/**
 * @file animated_model.cpp
 * @brief Implementation of the AnimatedModel class for skeletal animation
 */

#include "animated_model.hpp"
#include "assimpGLMHelpers.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <regex>

/**
 * @brief Decode URI-encoded strings from model files
 *
 * Handles common URI encodings found in GLTF and other model formats
 *
 * @param uri URI-encoded string
 * @return Decoded string with special characters restored
 */
static std::string decodeUri(const std::string &uri) {
  std::string out = uri;
  out = std::regex_replace(out, std::regex("%20"), " ");
  out = std::regex_replace(out, std::regex("%2B"), "+");
  out = std::regex_replace(out, std::regex("%28"), "(");
  out = std::regex_replace(out, std::regex("%29"), ")");
  out = std::regex_replace(out, std::regex("%5B"), "[");
  out = std::regex_replace(out, std::regex("%5D"), "]");
  return out;
}

/**
 * @brief Constructor for animated model
 * @param pos Initial world position
 * @param size Scale factor
 * @param noTex Whether to use materials instead of textures
 */
AnimatedModel::AnimatedModel(glm::vec3 pos, glm::vec3 size, bool noTex)
    : Model(pos, size, noTex) {}

/**
 * @brief Destructor
 * Base class destructor handles cleanup automatically
 */
AnimatedModel::~AnimatedModel() {
  // Base class destructor will be called automatically and handle Cleanup()
}

/**
 * @brief Load animated model from file
 *
 * Uses Assimp to load model data including meshes, materials, and bone
 * hierarchy. Supports various formats including GLTF, FBX, DAE, etc.
 *
 * @param path Path to the model file
 */
void AnimatedModel::LoadModel(std::string path) {
  Assimp::Importer import;
  const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "[ERROR::ANIMATED_MODEL::LOADING] Failed to load model at "
              << path << ": " << import.GetErrorString() << std::endl;
    return;
  }

  this->directory = path.substr(0, path.find_last_of("/"));
  ProcessNode(scene->mRootNode, scene);
}

/**
 * @brief Render the animated model
 *
 * Sets up model transformation matrix and renders all animated meshes.
 * Applies position, rotation, and scale transformations.
 *
 * @param shader Shader program with appropriate uniforms
 */
void AnimatedModel::Render(Shader &shader) {
  // Build model transformation matrix
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, this->pos);
  model = glm::rotate(model, this->rotation, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, this->size);

  // Set shader uniforms
  shader.SetMat4("model", model);
  shader.SetFloat("material.shininess", Config::Animation::DEFAULT_SHININESS);

  // Render all meshes
  for (AnimatedMesh &mesh : this->animatedMeshes) {
    mesh.Render(shader);
  }
}

/**
 * @brief Find bone ID by name
 * @param boneName Name of the bone to search for
 * @return Bone ID if found, -1 if not found
 */
int AnimatedModel::FindBoneID(const std::string &boneName) {
  auto it = this->boneInfoMap.find(boneName);
  if (it != this->boneInfoMap.end()) {
    return it->second.id;
  }
  return -1; // Not found
}

/**
 * @brief Process scene node hierarchy recursively
 *
 * Traverses the scene graph and processes all meshes in the hierarchy
 *
 * @param node Current node being processed
 * @param scene Assimp scene containing model data
 */
void AnimatedModel::ProcessNode(aiNode *node, const aiScene *scene) {
  // Process all meshes in this node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    this->animatedMeshes.push_back(ProcessMesh(mesh, scene));
  }

  // Recursively process child nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

/**
 * @brief Process individual mesh with animation data
 *
 * Extracts vertex data, indices, materials, textures, and bone weights
 * from an Assimp mesh and creates an AnimatedMesh object.
 *
 * @param mesh Assimp mesh to process
 * @param scene Assimp scene for material and texture access
 * @return Processed AnimatedMesh ready for rendering
 */
AnimatedMesh AnimatedModel::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<AnimatedVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Extract vertex data
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    AnimatedVertex vertex;
    SetVertexBoneDataToDefault(vertex);

    // Position
    vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                           mesh->mVertices[i].z);
    // Normal
    vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                              mesh->mNormals[i].z);

    // Texture coordinates (if available)
    if (mesh->mTextureCoords[0]) {
      vertex.texCoord =
          glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    } else {
      vertex.texCoord = glm::vec2(Config::Animation::DEFAULT_TEXCOORD);
    }

    vertices.push_back(vertex);
  }

  // Extract face indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Extract bone weight data
  ExtractBoneWeightForVertices(vertices, mesh, scene);

  // Process materials and textures
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    if (this->noTex) {
      // Use material colors only
      aiColor4D diff(1.0f), spec(1.0f);
      aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diff);
      aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &spec);
      return AnimatedMesh(vertices, indices, diff, spec);
    }

    // Lambda to load textures of a specific type
    auto loadTexType = [&](aiTextureType type) {
      std::vector<Texture> texs;
      for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        aiString str;
        material->GetTexture(type, i, &str);

        std::string filename = decodeUri(std::string(str.C_Str()));
        std::string fullPath = this->directory + "/" + filename;

        Texture tex(this->directory, filename, type);
        tex.Load(false);
        texs.push_back(std::move(tex));
      }
      return texs;
    };

    // Load diffuse textures
    auto diffuseMaps = loadTexType(aiTextureType_DIFFUSE);
    for (auto &tex : diffuseMaps) {
      textures.push_back(std::move(tex));
    }

    // Load specular textures
    auto specularMaps = loadTexType(aiTextureType_SPECULAR);
    for (auto &tex : specularMaps) {
      textures.push_back(std::move(tex));
    }
  }

  return AnimatedMesh(std::move(vertices), std::move(indices),
                      std::move(textures));
}

/**
 * @brief Initialize vertex bone data to defaults
 *
 * Sets all bone IDs to -1 and weights to 0.0
 *
 * @param vertex Vertex to initialize
 */
void AnimatedModel::SetVertexBoneDataToDefault(AnimatedVertex &vertex) {
  for (int i = 0; i < Config::Animation::MAX_BONE_INFLUENCE; i++) {
    vertex.boneIDs[i] = -1;
    vertex.weights[i] = 0.0f;
  }
}

/**
 * @brief Assign bone influence to a vertex
 *
 * Adds bone influence data to a vertex. If all slots are full,
 * replaces the influence with the smallest weight if the new weight is larger.
 *
 * @param vertex Vertex to modify
 * @param boneID ID of the influencing bone
 * @param weight Weight of the influence (0.0 to 1.0)
 */
void AnimatedModel::SetVertexBoneData(AnimatedVertex &vertex, int boneID,
                                      float weight) {
  // Find the slot with minimum weight
  int minIndex = 0;
  for (int i = 1; i < Config::Animation::MAX_BONE_INFLUENCE; i++) {
    if (vertex.weights[i] < vertex.weights[minIndex]) {
      minIndex = i;
    }
  }

  // Try to find an empty slot first
  for (int i = 0; i < Config::Animation::MAX_BONE_INFLUENCE; i++) {
    if (vertex.boneIDs[i] < 0) {
      vertex.boneIDs[i] = boneID;
      vertex.weights[i] = weight;
      return;
    }
  }

  // If no empty slot, replace the minimum weight if new weight is larger
  if (weight > vertex.weights[minIndex]) {
    vertex.boneIDs[minIndex] = boneID;
    vertex.weights[minIndex] = weight;
  }
}

/**
 * @brief Extract bone weight data from mesh
 *
 * Processes bone information from the mesh and assigns weights to vertices.
 * Only processes bones that match the allowed bone filters defined in config.
 *
 * @param vertices Vertex array to populate with bone data
 * @param mesh Source mesh containing bone information
 * @param scene Assimp scene (unused but kept for compatibility)
 */
void AnimatedModel::ExtractBoneWeightForVertices(
    std::vector<AnimatedVertex> &vertices, aiMesh *mesh, const aiScene *scene) {

  for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

    // Filter bones using Config::Animation::ALLOWED_BONES
    bool allowed = false;
    for (auto *filter : Config::Animation::ALLOWED_BONES) {
      // Check for DEF- prefix or exact match
      if ((filter[0] == 'D' && boneName.rfind(filter, 0) == 0) ||
          boneName == filter) {
        allowed = true;
        break;
      }
    }
    if (!allowed)
      continue;

    int boneID = -1;

    // Add new bone or get existing bone ID
    if (this->boneInfoMap.find(boneName) == this->boneInfoMap.end()) {
      BoneInfo newBoneInfo;
      newBoneInfo.id = this->boneCounter;
      newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
          mesh->mBones[boneIndex]->mOffsetMatrix);

      this->boneInfoMap[boneName] = newBoneInfo;
      boneID = this->boneCounter;
      this->boneCounter++;
    } else {
      boneID = this->boneInfoMap[boneName].id;
    }

    // Assign weights to vertices
    auto weights = mesh->mBones[boneIndex]->mWeights;
    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
      int vertexId = weights[weightIndex].mVertexId;
      float weight = weights[weightIndex].mWeight;

      SetVertexBoneData(vertices[vertexId], boneID, weight);
    }
  }
}

/**
 * @brief Calculate bounding box for collision detection
 *
 * Computes axis-aligned bounding box for the animated model
 * taking into account position, rotation, and scale transformations.
 */
void AnimatedModel::CalculateBoundingBox() {
  if (animatedMeshes.empty())
    return;

  // Build model transformation matrix
  glm::mat4 modelMat = glm::mat4(1.0f);
  modelMat = glm::translate(modelMat, this->pos);

  // Apply additional rotations
  if (additionalRotation.x != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.x, glm::vec3(1, 0, 0));
  if (additionalRotation.y != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.y, glm::vec3(0, 1, 0));
  if (additionalRotation.z != 0.0f)
    modelMat = glm::rotate(modelMat, additionalRotation.z, glm::vec3(0, 0, 1));

  // Apply main rotation and scale
  modelMat = glm::rotate(modelMat, this->rotation, glm::vec3(0, 1, 0));
  modelMat = glm::scale(modelMat, this->size);

  glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);

  // Process all vertices from all animated meshes
  for (const auto &mesh : animatedMeshes) {
    for (const auto &vertex : mesh.vertices) {
      glm::vec4 worldPos = modelMat * glm::vec4(vertex.pos, 1.0f);
      glm::vec3 v = glm::vec3(worldPos);
      minPos = glm::min(minPos, v);
      maxPos = glm::max(maxPos, v);
    }
  }

  // Store bounding box data
  boundingBox.min = minPos;
  boundingBox.max = maxPos;
  boundingBox.center = (minPos + maxPos) * 0.5f;

  // Calculate bounding sphere radius (using XZ plane)
  glm::vec3 extent = maxPos - minPos;
  boundingBox.radius = glm::max(extent.x, extent.z) * 0.5f;
}
