#include "staticmesh.h"

StaticMesh::~StaticMesh() {}

/// <summary>
/// Load a mesh with a given name.
/// </summary>
/// <param name="file_name">The full name of the model to load.</param>
/// <returns>A boolean. True if loading succeeds, false otherwise.</returns>
bool StaticMesh::loadMesh(std::string file_name, bool popBuffers) {
    populateBuffer = popBuffers;
    mesh_path = file_name;

    std::string rpath = MODELPATH(file_name) + file_name;
    scene = aiImportFile(
        rpath.c_str(),
        AI_LOAD_FLAGS);

    bool valid_scene = false;

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n%s", rpath.c_str(), aiGetErrorString());
        valid_scene = false;
    } else {
        valid_scene = initScene(scene, file_name);
    }

    glBindVertexArray(0);  // avoid modifying VAO between loads

    if (valid_scene) printf("Successfully loaded %sstatic mesh \"%s\"\n", popBuffers ? "" : "(unpopulated) ", name.c_str());
    return valid_scene;
}

bool StaticMesh::initScene(const aiScene* scene, std::string file_name) {
    meshes.resize(scene->mNumMeshes);
    materials.resize(scene->mNumMaterials);

    unsigned int nvertices = 0;
    unsigned int nindices = 0;

    // Count all vertices and indices
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;  // get current material index
        meshes[i].n_Indices = scene->mMeshes[i]->mNumFaces * 3;       // there are 3 times as many indices as there are faces (since they're all triangles)
        meshes[i].baseVertex = nvertices;                             // index of first vertex in the current mesh
        meshes[i].baseIndex = nindices;                               // track number of indices

        // Move forward by the corresponding number of vertices/indices to find the base of the next vertex/index
        nvertices += scene->mMeshes[i]->mNumVertices;
        nindices += meshes[i].n_Indices;
    }

    // Reallocate space for structure of arrays (SOA) values
    vertices.reserve(nvertices);
    normals.reserve(nvertices);
    texCoords.reserve(nvertices);
    indices.reserve(nindices);

    // Initialise meshes
    for (unsigned int i = 0; i < meshes.size(); i++) {
        const aiMesh* am = scene->mMeshes[i];
        initSingleMesh(am);
    }

    if (!initMaterials(scene, file_name)) {
        return false;
    }

    if (populateBuffer) populateBuffers();
    return glGetError() == GL_NO_ERROR;
}

/// <summary>
/// Initialise a single mesh object and add its values (vertices, indices, positions, normals, and texture coordinates) to the parent mesh.
/// </summary>
/// <param name="amesh">The mesh to initialise.</param>
void StaticMesh::initSingleMesh(const aiMesh* amesh) {
    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < amesh->mNumVertices; i++) {
        const aiVector3D& pPos = amesh->mVertices[i];
        const aiVector3D& pNormal = amesh->mNormals ? amesh->mNormals[i] : aiVector3D(0.0f, 1.0f, 0.0f);
        const aiVector3D& pTexCoord = amesh->HasTextureCoords(0) ? amesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);

        vertices.push_back(vec3(pPos.x, pPos.y, pPos.z));
        normals.push_back(vec3(pNormal.x, pNormal.y, pNormal.z));
        texCoords.push_back(vec2(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < amesh->mNumFaces; i++) {
        const aiFace& Face = amesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }
}

/// <summary>
/// Initialise the materials and textures used in the mesh.
/// </summary>
/// <param name="scene">The mesh to load.</param>
/// <param name="file_name">The name of the model file.</param>
/// <returns>A boolean. <code>true</code> if everything loaded correctly, false otherwise.</returns>
bool StaticMesh::initMaterials(const aiScene* scene, std::string file_name) {
    std::string dir = MODELPATH(file_name);
    std::vector<std::string> paths;
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = scene->mMaterials[i];

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* embeddedTex = scene->GetEmbeddedTexture(Path.C_Str());
                if (embeddedTex) {
                    materials[i].diffTex = new Texture(GL_TEXTURE_2D);
                    unsigned int buffer = embeddedTex->mWidth;
                    materials[i].diffTex->load(buffer, embeddedTex->pcData);
                    printf("%s: embedded diffuse texture type %s\n", name.c_str(), embeddedTex->achFormatHint);
                } else {
                    std::string p(Path.data);
                    // std::cout << p << std::endl;
                    if (p.substr(0, 2) == ".\\") {
                        p = p.substr(2, p.size() - 2);
                    }
                    std::string fullPath = dir + p;
                    materials[i].diffTex = new Texture(GL_TEXTURE_2D_ARRAY);
                    if (materials[i].diffTex->loadAtlas(fullPath, atlasTileSize, atlasTilesUsed)) {
                        // printf("%s: Loaded diffuse texture '%s'\n", name.c_str(), fullPath.c_str());
                    } else {
                        printf("Error loading diffuse texture '%s'\n", fullPath.c_str());
                        return false;
                    }
                }
            }
        }

        if (pMaterial->GetTextureCount(aiTextureType_METALNESS) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* embeddedTex = scene->GetEmbeddedTexture(Path.C_Str());
                if (embeddedTex) {
                    materials[i].mtlsTex = new Texture(GL_TEXTURE_2D);
                    unsigned int buffer = embeddedTex->mWidth;
                    materials[i].mtlsTex->load(buffer, embeddedTex->pcData);
                    printf("%s: embedded metalness texture type %s\n", name.c_str(), embeddedTex->achFormatHint);
                } else {
                    std::string p(Path.data);
                    // std::cout << p << std::endl;
                    if (p.substr(0, 2) == ".\\") {
                        p = p.substr(2, p.size() - 2);
                    }
                    std::string fullPath = dir + p;
                    materials[i].mtlsTex = new Texture(GL_TEXTURE_2D_ARRAY);
                    if (materials[i].mtlsTex->loadAtlas(fullPath, atlasTileSize, atlasTilesUsed)) {
                        // printf("%s: Loaded metalness texture '%s'\n", name.c_str(), fullPath.c_str());
                    } else {
                        printf("Error loading metalness texture '%s'\n", fullPath.c_str());
                        return false;
                    }
                }
            }
        }
    }

    return glGetError() == GL_NO_ERROR;
}

/// <summary>
/// Bind and enable the VAO, VBOs, and EBO for usage.
/// </summary>
void StaticMesh::populateBuffers() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &p_VBO);
    glGenBuffers(1, &n_VBO);
    glGenBuffers(1, &t_VBO);
    glGenBuffers(1, &d_VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &IBO);

    glBindBuffer(GL_ARRAY_BUFFER, p_VBO);
    if (useCustomVertices) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertices.size(), NULL, GL_STATIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_DYNAMIC_DRAW);
    }
    glEnableVertexAttribArray(ST_POSITION_LOC);
    glVertexAttribPointer(ST_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, n_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(ST_NORMAL_LOC);
    glVertexAttribPointer(ST_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, t_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(ST_TEXTURE_LOC);
    glVertexAttribPointer(ST_TEXTURE_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, IBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * SM::MAX_NUM_INSTANCES, NULL, GL_DYNAMIC_DRAW);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(ST_INSTANCE_LOC + i);
        glVertexAttribPointer(ST_INSTANCE_LOC + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (const void*)(i * sizeof(vec4)));
        glVertexAttribDivisor(ST_INSTANCE_LOC + i, 1);  // tell OpenGL this is an instanced vertex attribute.
    }

    glBindBuffer(GL_ARRAY_BUFFER, d_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * SM::MAX_NUM_INSTANCES, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(ST_DEPTH_LOC);
    glVertexAttribPointer(ST_DEPTH_LOC, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
    glVertexAttribDivisor(ST_DEPTH_LOC, 1);  // tell OpenGL this is an instanced vertex attribute.
}

void StaticMesh::setCustomVertices(std::vector<vec3> vs) { 
    assert(vs.size() == vertices.size() && "attempted to set invalid amount of custom vertices");
    cVertices = vs; 
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This function supports instancing and atlas coordinates.
/// </summary>
/// <param name="nInstances">The number of instances you would like to draw.</param>
/// <param name="model_matrix">The matrices you would like to transform each instance with.</param>
void StaticMesh::render(unsigned int nInstances, const mat4* model_matrix, const float* atlasDepths) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, IBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * nInstances, &model_matrix[0]);
    if (useCustomVertices) {
        glBindBuffer(GL_ARRAY_BUFFER, p_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * cVertices.size(), &cVertices[0]);
    } else {
        // in case useCustomVertices is disabled after being enabled
        glBindBuffer(GL_ARRAY_BUFFER, p_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * vertices.size(), &vertices[0]);
    }
    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int mIndex = meshes[i].materialIndex;
        assert(mIndex < materials.size());

        glBindBuffer(GL_ARRAY_BUFFER, d_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * nInstances, &atlasDepths[0]);
        if (materials[mIndex].diffTex) materials[mIndex].diffTex->bind(GL_TEXTURE0);
        if (materials[mIndex].mtlsTex) materials[mIndex].mtlsTex->bind(GL_TEXTURE1);

        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            meshes[i].n_Indices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].baseIndex),
            nInstances,
            meshes[i].baseVertex);

        // unbind textures so they don't "spill over"
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
    glBindVertexArray(0);  // prevent VAO from being changed externally
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This function supports instancing.
/// </summary>
/// <param name="nInstances">The number of instances you would like to draw.</param>
/// <param name="model_matrix">The matrices you would like to transform each instance with.</param>
void StaticMesh::render(unsigned int nInstances, const mat4* model_matrix) {
    std::vector<float> dpths(nInstances, 0);
    render(nInstances, model_matrix, dpths.data());
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This is a special case that will render exactly one instance of your mesh.
/// </summary>
/// <param name="mat">The transform you would like to apply to your instance.</param>
void StaticMesh::render(mat4 mat, float depth) {
    this->mat = mat;
    const float ds[1] = {depth};
    render(1, &mat, ds);
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This is a special case that will render exactly one instance of your mesh.
/// </summary>
/// <param name="mat">The transform you would like to apply to your instance.</param>
void StaticMesh::render(mat4 mat) {
    render(mat, 0);
}
