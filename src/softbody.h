#ifndef SOFTBODY_H
#define SOFTBODY_H

#include <fstream>
#include <iostream>

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
        // mesh->useCustomVertices = true; // enable use of custom vertices
        mesh->loadMesh(meshPath);
        loadTetraFile();
    }

    void loadTetraFile() {
        tetraPath = TETRAPATH(mesh->mesh_path);
        std::ifstream file(tetraPath);
        if (!file.is_open()) {
            std::cout << "Failed to open file " << tetraPath << std::endl;
            return;
        }

        std::string line;
        int idx = 1;
        int vs = 0, es = 0, fs = 0, ts = 0, tns = 0;
        while (getline(file, line)) {
            std::vector<std::string> vals = Util::split(line, " ");
            if (idx == 1) {
                vs = stoi(vals[1]);
                vertices.reserve(vs);
            } else if (idx == 2) {
                es = stoi(vals[1]);
                edges.reserve(es);
            } else if (idx == 3) {
                fs = stoi(vals[1]);
                faces.reserve(fs);
            } else if (idx == 4) {
                ts = stoi(vals[1]);
                tetras.reserve(ts);
            } else if (idx == 5) {
                tns = stoi(vals[1]);
                tetraNeighbours.reserve(tns);
            } else {
                std::string type = vals[0];
                if (type == "v") {
                    vertices.push_back({
                        stof(vals[1]),
                        stof(vals[2]),
                        stof(vals[3]),
                    });
                } else if (type == "e") {
                    edges.push_back({
                        stoi(vals[1]),
                        stoi(vals[2]),
                    });
                } else if (type == "f") {
                    faces.push_back({
                        stoi(vals[1]),
                        stoi(vals[2]),
                        stoi(vals[3]),
                    });
                } else if (type == "t") {
                    tetras.push_back({
                        stoi(vals[1]),
                        stoi(vals[2]),
                        stoi(vals[3]),
                        stoi(vals[4]),
                    });
                } else if (type == "tn") {
                    tetraNeighbours.push_back({
                        stoi(vals[1]),
                        stoi(vals[2]),
                        stoi(vals[3]),
                        stoi(vals[4]),
                    });
                }
            }

            idx++;
        }
        assert(vertices.size() == vs && "mismatched vertex count");
        assert(edges.size() == es && "mismatched edges count");
        assert(faces.size() == fs && "mismatched faces count");
        assert(tetras.size() == ts && "mismatched tetrahedra count");
        assert(tetraNeighbours.size() == tns && "mismatched tetra neighbour count");

        file.close();
        printf("Successfully loaded tetrahedral mesh \"%s.tetra\"\n", MODEL_NO_DIR(mesh->mesh_path).c_str());
        return;
    }

    void update() {

    }

    using Edge = ivec2;
    using Face = ivec3;
    using Tetra = std::tuple<unsigned, unsigned, unsigned, unsigned>;
    StaticMesh* mesh;
    std::vector<vec3> vertices;
    std::vector<Edge> edges;
    std::vector<Face> faces;
    std::vector<Tetra> tetras;
    std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> tetraNeighbours;

    std::string name;
    std::string tetraPath;
};


#endif /* SOFTBODY_H */
