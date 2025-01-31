#ifndef STATICMESH_H
#define STATICMESH_H

#pragma warning(disable : 26495)

#include "mesh.h"

#define AI_LOAD_FLAGS aiProcess_Triangulate | aiProcess_PreTransformVertices

class StaticMesh : public Mesh {
   public:
    // Create a new Mesh object without a mesh
    StaticMesh() { name = "NewMesh" + std::to_string(SM::unnamedStaticMeshCount++); }

    // Create a new unnamed Mesh object
    StaticMesh(std::string file_name) {
        name = "NewMesh" + std::to_string(SM::unnamedStaticMeshCount++);
        if (!loadMesh(file_name)) std::cout << "\n\nfailed to load mesh \"" << file_name.c_str() << "\" :(\n";
    }

    // Create a new Mesh object and give it a name.
    StaticMesh(std::string nm, std::string file_name) {
        name = nm;
        if (!loadMesh(file_name)) std::cout << "\n\nfailed to load mesh \"" << nm.c_str() << "\" :(\n";
    }

    // Create a new Mesh object from a atlas texture.
    // The width of the texture must be `_atlasTileSize` and the length must be at least `_atlasTileSize` * `_atlasTilesUsed`
    // If `isVerticalTexture` is `true`, the texture is assumed to be one tile wide and `_atlasTilesUsed` tiles long.
    // If `isVerticalTexture` is `false`, it is assumed the texture only uses tiles of size `_atlasTileSize`.
    StaticMesh(std::string nm, std::string file_name, int _atlasTileSize, int _atlasTilesUsed, bool load = true) {
        name = nm;
        usingAtlas = true;
        atlasTileSize = _atlasTileSize;
        atlasTilesUsed = _atlasTilesUsed;
        if (load)
            if (!loadMesh(file_name)) std::cout << "\n\nfailed to load mesh \"" << nm.c_str() << "\" :(\n";
    }

    ~StaticMesh();

    bool loadMesh(std::string file_name) { return loadMesh(file_name, true); }
    bool loadMesh(std::string file_name, bool popBuffer);
    bool initScene(const aiScene*, std::string);
    void initSingleMesh(const aiMesh*);
    bool initMaterials(const aiScene*, std::string);
    void populateBuffers();
    void setCustomVertices(std::vector<vec3> vs);
    void render(unsigned int, const mat4*);                // render an array of meshes using instancing
    void render(unsigned int, const mat4*, const float*);  // render an array of meshes using instancing and atlas depths
    void render(mat4, float);                              // single atlas depth
    void render(mat4);                                     // render a single mesh

   private:
    std::vector<mat4> getUpdatedTransforms(Shader* skinnedShader, float animSpeed) { return {}; }  // not implemented
    std::vector<mat4> getUpdatedTransforms(float animSpeed) { return {}; }                         // not implemented
    void update() {}                                                                               // not implemented
    void update(Shader* shader) {}                                                                 // not implemented
    void update(float speed) {}                                                                    // not implemented
    void update(Shader* shader, float speed) {}                                                    // not implemented

#define ST_POSITION_LOC 0  // p_vbo
#define ST_NORMAL_LOC 1    // n_vbo
#define ST_TEXTURE_LOC 2   // t_vbo
#define ST_INSTANCE_LOC 3
#define ST_DEPTH_LOC 7  // texture depth
};

#endif /* STATICMESH_H */
