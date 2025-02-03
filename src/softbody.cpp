#include "softbody.h"

void SoftBody::loadTetraFile() {
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
            // invMasses.reserve(vs);
            // velocities.reserve(vs);
        } else if (idx == 2) {
            es = stoi(vals[1]);
            edges.reserve(es);
        } else if (idx == 3) {
            fs = stoi(vals[1]);
            // faces.reserve(fs); // !
        } else if (idx == 4) {
            ts = stoi(vals[1]);
            tetras.reserve(ts);
        } else if (idx == 5) {
            tns = stoi(vals[1]);
            tetraNeighbours.reserve(tns);
        } else {
            std::string type = vals[0];
            if (type == "v") {
                vertices.emplace_back(
                    vertices.size(),
                    vec3(stof(vals[1]),
                    stof(vals[2]),
                    stof(vals[3]))
                );
            } else if (type == "e") {
                edges.emplace_back(
                    edges.size(),
                    stoi(vals[1]),
                    stoi(vals[2])
                );
            } else if (type == "f") {
                // faces.push_back({
                //     stoi(vals[1]),
                //     stoi(vals[2]),
                //     stoi(vals[3]),
                // });
            } else if (type == "t") {
                tetras.emplace_back(
                    tetras.size(),
                    stoi(vals[1]),
                    stoi(vals[2]),
                    stoi(vals[3]),
                    stoi(vals[4])
                );
            } else if (type == "tn") {
                tetraNeighbours.emplace_back(
                    stoi(vals[1]),
                    stoi(vals[2]),
                    stoi(vals[3]),
                    stoi(vals[4])
                );
            }
        }

        idx++;
    }
    assert(vertices.size() == vs && "mismatched vertex count");
    assert(edges.size() == es && "mismatched edges count");
    // assert(faces.size() == fs && "mismatched faces count");
    assert(tetras.size() == ts && "mismatched tetrahedra count");
    assert(tetraNeighbours.size() == tns && "mismatched tetra neighbour count");

    file.close();
    printf("Successfully loaded tetrahedral mesh \"%s.tetra\"\n", MODEL_NO_DIR(mesh->mesh_path).c_str());
    printf("%d vs, %d es, %d ts\n", vs, es, ts);
    return;
}

long long SoftBody::getHashKey(ivec3 cell) {
    long long s = (cell.x * 6096427489LL) + (cell.y * 4039848257LL) + (cell.z * 5993801789LL);
    return s;
}

ivec3 SoftBody::getCellCoord(vec3 p) {
    return ivec3(floor(p.x / cellSize), floor(p.y / cellSize), floor(p.z / cellSize));
}

// populate the cell to mesh map
void SoftBody::initHash() {
    std::for_each(std::execution::par, mvIndices.begin(), mvIndices.end(), [&](auto&& i) {
        vec3 mv = mesh->vertices[i];
        ivec3 cell = getCellCoord(mv);
        long long key = getHashKey(cell);
        cellToVis[key].push_back(i);
    });
}

// query all visual mesh vertices near the point `p` within a radius `r`
void SoftBody::queryNearbyMV(vec3 p, float r) {
    querySize = 0;
    queryIDs.clear();
    ivec3 lcell = getCellCoord(p - r);
    ivec3 hcell = getCellCoord(p + r);
    for (int x = lcell.x; x <= hcell.x; ++x) {
        for (int y = lcell.y; y <= hcell.y; ++y) {
            for (int z = lcell.z; z <= hcell.z; ++z) {
                ivec3 cell = {x, y, z};             // create cell
                auto key = getHashKey(cell); // get cell hash
                for (auto id: cellToVis[key]) {
                    if (auto [it, ins] = queryIDs.insert(id); !ins) printf("collision"); // debug
                }
            }
        }
    }
}

void SoftBody::initPhysics() {
    previousPositions.resize(vertices.size());
    for (auto& tet : tetras) {
        float vol = computeTetraVolume(tet.tID);
        tet.restVolume = vol;
        float pinvMass = vol > 0 ? 1.f / (vol / 4.f) : 0;
        vertices[tet.x1].invMass += pinvMass;
        vertices[tet.x2].invMass += pinvMass;
        vertices[tet.x3].invMass += pinvMass;
        vertices[tet.x4].invMass += pinvMass;
    }
    for (auto& e : edges) {
        e.restLength = distance(vertices[e.x1].position, vertices[e.x2].position);
    }
}

// calculate the volume of the tetrahedra `i`
float SoftBody::computeTetraVolume(int i) {
    Tetra tet = tetras[i];
    float f = 1.f / 6;
    vec3 x21 = vertices[tet.x2].position - vertices[tet.x1].position;
    vec3 x31 = vertices[tet.x3].position - vertices[tet.x1].position;
    vec3 x41 = vertices[tet.x4].position - vertices[tet.x1].position;
    vec3 c_21_31 = cross(x21, x31);
    return f * dot(c_21_31, x41);
}

// calculate the volume of four points
float SoftBody::computeTetraVolume(vec3 p1, vec3 p2, vec3 p3, vec3 p4) {
    float f = 1.f / 6;
    vec3 x21 = p2 - p1;
    vec3 x31 = p3 - p1;
    vec3 x41 = p4 - p1;
    vec3 c_21_31 = cross(x21, x31);
    return f * dot(c_21_31, x41);
}

// create a hashed position for each mesh vertex
void SoftBody::computeSkinningInfo() {
    // for (const auto& [key, value] : cellToVis)
    //     std::cout << '[' << key << "] = " << value.size() << "; " << std::endl;
    const std::vector<vec3>& mvs = mesh->vertices;
    std::vector<float> minDist(mVertexCount, std::numeric_limits<float>::max());
    vec3 tCentre = vec3(0);
    mat3 P = mat3(0);
    vec4 bary = vec4(0);
    int cnt = 0;
    std::for_each(std::execution::par, tetras.begin(), tetras.end(), [&](auto&& tet) {
        vec3 p1 = vertices[tet.x1].position;
        vec3 p2 = vertices[tet.x2].position;
        vec3 p3 = vertices[tet.x3].position;
        vec3 p4 = vertices[tet.x4].position;
        tCentre = vec3(0);
        // tCentre is avg of coords
        tCentre += (p1 + p2 + p3 + p4) * 0.25f;

        // find the largest radius that encompasses all tetrahedron points
        float maxRadius = 0;
        maxRadius = max(maxRadius, distance(p1, tCentre));
        maxRadius = max(maxRadius, distance(p2, tCentre));
        maxRadius = max(maxRadius, distance(p3, tCentre));
        maxRadius = max(maxRadius, distance(p4, tCentre));
        maxRadius += cellSize;

        queryNearbyMV(tCentre, maxRadius);
        if (queryIDs.empty()) return;

        // create matrix from vertices, subtracting the contrained point `p4`
        vec3 x14 = p1 - p4;
        vec3 x24 = p2 - p4;
        vec3 x34 = p3 - p4;
        P = mat3(x14, x24, x34);
        P = inverse(P); // v - p4 = Pb ==> b = inv(P)(v - p4)

        std::for_each(queryIDs.begin(), queryIDs.end(), [&](auto&& id) {
            int mID = id; // visual mesh vertex ID
            vec3 v = mesh->vertices[mID];

            if (minDist[mID] <= 0) return; // already have skinning info (dist is negative or 0)
            if (distance(v, tCentre) > maxRadius) return; // outside search radius
            cnt++;
            // compute barycentric coordinates
            vec3 vp_4 = v - p4;
            vec3 b = P * vp_4;
            bary = vec4(b, 1 - b.x - b.y - b.z);
            
            // compute max min dists and update the closest tetrahedron for visual mesh vertex `i`
            float dst = 0;
            dst = max(dst, -bary.x);
            dst = max(dst, -bary.y);
            dst = max(dst, -bary.z);
            dst = max(dst, -bary.w);
            if (dst < minDist[mID]) {
                minDist[mID] = dst;
                tetraMap[mID] = {tet.tID, b};
            }
        });
    });
}

void SoftBody::solveEdgeConstraint() {
    std::for_each(std::execution::par, edges.begin(), edges.end(), [&](auto&& e) {
        if (vertices[e.x1].invMass + vertices[e.x2].invMass == 0) return;
        vec3 offset_i = vertices[e.x1].position - vertices[e.x2].position;
        vec3 offset_j = -offset_i;
        float l = length(offset_i);
        float C = l - e.restLength;
        vec3 dxi = normalize(offset_i);  // constraint gradient for i
        vec3 dxj = normalize(offset_j);  // constraint gradient for j
        float denom = (vertices[e.x1].invMass) +
                      (vertices[e.x2].invMass) +
                      (edgeCompliance / (sdt * sdt));
        // denom += 1e-3f;  // to avoid division by 0. also needs the timestep needs to be low enough to avoid NaN values
        float lambda = -C / denom;
        vertices[e.x1].position += lambda * vertices[e.x1].invMass * dxi;
        vertices[e.x2].position += lambda * vertices[e.x2].invMass * dxj;
    });
}

void SoftBody::solveVolumeConstraint() {
    float alpha = volumeCompliance / sdt / sdt;
    std::for_each(std::execution::par, tetras.begin(), tetras.end(), [&](auto&& tet) {
        Vertex v1 = vertices[tet.x1];
        Vertex v2 = vertices[tet.x2];
        Vertex v3 = vertices[tet.x3];
        Vertex v4 = vertices[tet.x4];
        vec3 grad1 = cross(v4.position - v2.position, v3.position - v2.position) * 1.f/6.f; // constraint gradient for vertex v1
        vec3 grad2 = cross(v3.position - v1.position, v4.position - v1.position) * 1.f/6.f; // constraint gradient for vertex v2
        vec3 grad3 = cross(v4.position - v1.position, v2.position - v1.position) * 1.f/6.f; // constraint gradient for vertex v3
        vec3 grad4 = cross(v2.position - v1.position, v3.position - v1.position) * 1.f/6.f; // constraint gradient for vertex v4
        float denom = v1.invMass * length2(grad1);
        denom += v2.invMass * length2(grad2);
        denom += v3.invMass * length2(grad3);
        denom += v4.invMass * length2(grad4);
        if (denom == 0) return;
        denom += alpha;
        float vol = computeTetraVolume(v1.position, v2.position, v3.position, v4.position);
        float C = vol - tet.restVolume;
        float lambda = -C / denom;
        vertices[tet.x1].position += lambda * v1.invMass * grad1;
        vertices[tet.x2].position += lambda * v2.invMass * grad2;
        vertices[tet.x3].position += lambda * v3.invMass * grad3;
        vertices[tet.x4].position += lambda * v4.invMass * grad4;
    });
}

void SoftBody::updateVisualMesh() {
    std::for_each(std::execution::par, mvIndices.begin(), mvIndices.end(), [&](auto&& i) {
        auto [tID, b] = tetraMap[i];
        vec4 bary = vec4(b, 1 - b.x - b.y - b.z);
        mesh->vertices[i] = 
            (vertices[tetras[tID].x1].position * bary.x) + 
            (vertices[tetras[tID].x2].position * bary.y) + 
            (vertices[tetras[tID].x3].position * bary.z) + 
            (vertices[tetras[tID].x4].position * bary.w);
    });
}

void SoftBody::applyForces() {
    std::for_each(std::execution::par, vertices.begin(), vertices.end(), [&](auto&& v) {
        if (v.invMass == 0) return;
        v.velocity += Util::DOWN * gravity * sdt;
    });
}

void SoftBody::constrainBounds() {
    for (auto& v : vertices) {
        if (v.position.y < floorY) v.position.y = floorY;
    }
}

void SoftBody::update() {
    sdt = dt / substeps;
    for (int i = 0; i < substeps; ++i) {
        applyForces();
        std::for_each(std::execution::par, tvIndices.begin(), tvIndices.end(), [&](auto&& i) {
            if (vertices[i].invMass == 0) return;
            previousPositions[i] = vertices[i].position;
            vertices[i].position += vertices[i].velocity * sdt;
        });
        solveEdgeConstraint();
        solveVolumeConstraint();
        std::for_each(std::execution::par, tvIndices.begin(), tvIndices.end(), [&](auto&& i) {
            if (vertices[i].invMass == 0) return;
            vertices[i].velocity = (vertices[i].position - previousPositions[i]) / sdt;
        });
        constrainBounds();
    }
    updateVisualMesh();
}
