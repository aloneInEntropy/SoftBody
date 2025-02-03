#ifndef SOFTBODY_H
#define SOFTBODY_H

#include <fstream>
#include <iostream>
#include <execution>
#include <set>
#include <list>

#include "util.h"
#include "staticmesh.h"

#define TETRAPATH(m) MODELPATH(m) + "Tetra/" + MODEL_NO_DIR(m) + ".tetra"

class SoftBody {
   public:
    SoftBody(std::string nm, StaticMesh* mesh_) {
        name = nm;
        mesh = mesh_;
        loadTetraFile();
    }
    SoftBody(std::string nm, std::string meshPath) {
        name = nm;
        mesh = new StaticMesh();
        mesh->name = nm + "_Static";
        mesh->useCustomVertices = true; // enable use of custom vertices
        mesh->loadMesh(meshPath);
        loadTetraFile();

        tetraCount = tetras.size();
        tVertexCount = vertices.size();
        mVertexCount = mesh->vertices.size();
        tvIndices.resize(tVertexCount);
        mvIndices.resize(mVertexCount);
        tetraMap.resize(mVertexCount);
        std::iota(tvIndices.begin(), tvIndices.end(), 0);  // set to 0, 1, 2, ..., tVertexCount
        std::iota(mvIndices.begin(), mvIndices.end(), 0);  // set to 0, 1, 2, ..., mVertexCount
        initHash();
        initPhysics();
        computeSkinningInfo();
        bounds = {50, 50, 50};
    }

    void loadTetraFile();
    float computeTetraVolume(int t);
    float computeTetraVolume(vec3 p1, vec3 p2, vec3 p3, vec3 p4);
    void update();
    void computeSkinningInfo();
    long long getHashKey(ivec3 cell);
    ivec3 getCellCoord(vec3 p);
    void queryNearbyMV(vec3 p, float r);
    void initHash();
    void initPhysics();
    void applyForces();
    void constrainBounds();
    void solveEdgeConstraint();
    void solveVolumeConstraint();
    void updateVisualMesh();

    struct Vertex {
        int vID;
        vec3 position;
        vec3 velocity = vec3(0);
        float invMass = 0;
        Vertex(int id, vec3 pos) : vID(id), position(pos) {}
    };

    struct Edge {
        int eID;
        int x1, x2;
        float restLength = 0;  // edge length
        Edge(int id, int a, int b) : eID(id), x1(a), x2(b) {}
    };

    struct Tetra {
        int tID;
        int x1, x2, x3, x4;
        float restVolume = 0;  // rest volume
        Tetra(int id, int w, int x, int y, int z) : tID(id), x1(w), x2(x), x3(y), x4(z) {}
    };

    StaticMesh* mesh;                            // mesh to base the soft body from
    std::vector<Vertex> vertices;                // tetrahedra vertices
    std::vector<Edge> edges;                     // tetrahedra edges
    std::vector<Tetra> tetras;                   // tetrahedra
    std::vector<std::pair<int, vec3>> tetraMap;  // mapping of visual mesh vertices to tetrahedra IDs and their (3D) barycentric coords
    std::vector<std::tuple<int, int, int, int>> tetraNeighbours;

    float edgeCompliance = 1;
    float volumeCompliance = 0;
    float cellSize = 0.1;  // grid size for particles. in 3D, particles are single points rather than spheres with radii
    float dt = 1.f / 120;
    float sdt = dt / substeps;
    int substeps = 10;
    float gravity = 0;
    int tVertexCount = 0;  // tetrahedral mesh vertex count
    int mVertexCount = 0;  // visual mesh vertex count
    int tetraCount = 0;    // tetrahedra count
    vec3 bounds;
    float floorY = 0;

    std::vector<int> tvIndices;  // indices of tetrahedral mesh
    std::vector<int> mvIndices;  // indices of visual mesh

    /* Hash variables */
    int tableSize = 0;
    int querySize = 0;
    std::set<int> queryIDs;  // temporary buffer for querying nearby visual mesh particles
    std::map<long long, std::list<int>> cellToVis; // sparse mapping of grid cell hashes to visual mesh vertex IDs
    std::vector<vec3> previousPositions; // previous positions of tetrahedral vertices

    std::string name;
    std::string tetraPath;
};

#endif /* SOFTBODY_H */
