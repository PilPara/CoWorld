#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include "assimp/material.h"
#include "mesh.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "config.hpp"

class Plane : public Model {
public:
  glm::vec3 m_pos;
  glm::vec3 m_size;

  Plane(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f))
      : Model(pos, size) {}

  void Init() {
    using namespace Config::Plane;

    int nVertices = 6;

    float vertices[] = {
        -SIZE, 0.0f, -SIZE, 0.0f, 1.0f, 0.0f, 0.0f,      0.0f,      // BL
        -SIZE, 0.0f, SIZE,  0.0f, 1.0f, 0.0f, 0.0f,      TEX_SCALE, // TL
        SIZE,  0.0f, SIZE,  0.0f, 1.0f, 0.0f, TEX_SCALE, TEX_SCALE, // TR
        -SIZE, 0.0f, -SIZE, 0.0f, 1.0f, 0.0f, 0.0f,      0.0f,      // BL
        SIZE,  0.0f, SIZE,  0.0f, 1.0f, 0.0f, TEX_SCALE, TEX_SCALE, // TR
        SIZE,  0.0f, -SIZE, 0.0f, 1.0f, 0.0f, TEX_SCALE, 0.0f       // BR
    };

    std::vector<unsigned int> indices(nVertices);
    for (unsigned int i = 0; i < nVertices; i++)
      indices[i] = i;

    std::vector<Vertex> verts;
    for (int i = 0; i < nVertices; i++) {
      Vertex v;
      v.pos = glm::vec3(vertices[i * VERTEX_FLOATS + 0],
                        vertices[i * VERTEX_FLOATS + 1],
                        vertices[i * VERTEX_FLOATS + 2]);
      v.normal = glm::vec3(vertices[i * VERTEX_FLOATS + 3],
                           vertices[i * VERTEX_FLOATS + 4],
                           vertices[i * VERTEX_FLOATS + 5]);
      v.texCoord = glm::vec2(vertices[i * VERTEX_FLOATS + 6],
                             vertices[i * VERTEX_FLOATS + 7]);
      verts.push_back(v);
    }

    Texture grass("assets", TEXTURE_FILE, aiTextureType_DIFFUSE);
    grass.Load(false);

    std::vector<Texture> textures;
    textures.push_back(std::move(grass));

    meshes.push_back(Mesh(verts, indices, std::move(textures)));
  }
};

#endif // PLANE_H
