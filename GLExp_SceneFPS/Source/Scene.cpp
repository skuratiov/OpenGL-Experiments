#include "framework.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "VertexBufferObjectIndirect.h"
#include "Image.h"
#include "Scene.h"

//
//  Globals
//
Scene* Scene::m_pInstance = nullptr;

struct VertexKey {
    int vi, ti, ni;
    bool operator==(const VertexKey& rhs) const {
        return vi == rhs.vi && ti == rhs.ti && ni == rhs.ni;
    }
};

struct VertexKeyHash {
    size_t operator()(const VertexKey& k) const {
        return ((size_t)k.vi * 73856093) ^ ((size_t)k.ti * 19349663) ^ ((size_t)k.ni * 83492791);
    }
};

//
// Constructor / destructor
//
Scene::Scene() {
    for (size_t i = 0; i < MAX_MESHES; ++i) {
        m_Meshes[i] = nullptr;
    }
	m_nMeshCount = 0;
}

Scene::~Scene() {
    std::unordered_set<Material*> uniqueMaterials;

	for (size_t i = 0; i < m_nMeshCount; ++i) {
        if (m_Meshes[i]->vbo) {
            m_Meshes[i]->vbo->deleteIndirect();
			delete m_Meshes[i]->vbo;
			m_Meshes[i]->vbo = nullptr;
        }

        if (m_Meshes[i]->material)
            uniqueMaterials.insert(m_Meshes[i]->material);

        delete m_Meshes[i];
		m_Meshes[i] = nullptr;
	}

    for (Material* m : uniqueMaterials) {
        if (m->diffuseTex) { delete m->diffuseTex; }
        if (m->normalMap) { delete m->normalMap; }
        delete m;
    }

	m_nMeshCount = 0;
}

BOOL Scene::fromOBJ(const char * lpFileName) {
	std::ifstream file(lpFileName, std::ios::binary | std::ios::ate);

	if (!file) return -1;

	const size_t fileSize = file.tellg();
	file.seekg(0);

	uint8_t* buffer = new uint8_t[fileSize];

	file.read(reinterpret_cast<char*>(buffer), fileSize);
	if (!file) return -2;

	file.close();

    int result = parseOBJ(buffer, fileSize);

	delete[] buffer; buffer = nullptr;

	return result;
}
int Scene::parseOBJ(uint8_t* buffer, size_t size) {
    std::istringstream iss(std::string(reinterpret_cast<char*>(buffer), size));
    std::string line;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    struct TempMesh {
        std::string materialName;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;
        std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;
    };

    std::vector<TempMesh> parsedMeshes;
    std::string currentMaterial = "default";
    bool hasActiveGeometry = false;

    auto startNewMesh = [&](const std::string& matName) {
        TempMesh newMesh;
        newMesh.materialName = matName;
        parsedMeshes.push_back(newMesh);
        hasActiveGeometry = false;
        };

    startNewMesh(currentMaterial);

    auto addVertexToMesh = [&](TempMesh& mesh, int vi, int ti, int ni) -> uint32_t {
        if (vi > 0) vi--; else if (vi < 0) vi = (int)positions.size() + vi;
        if (ti > 0) ti--; else if (ti < 0) ti = (int)uvs.size() + ti;
        if (ni > 0) ni--; else if (ni < 0) ni = (int)normals.size() + ni;

        VertexKey key{ vi, ti, ni };
        auto it = mesh.vertexMap.find(key);
        if (it != mesh.vertexMap.end()) return it->second;

        uint32_t newIdx = (uint32_t)mesh.vertices.size();
        mesh.vertices.push_back((vi >= 0 && vi < (int)positions.size()) ? positions[vi] : glm::vec3(0.0f));
        mesh.normals.push_back((ni >= 0 && ni < (int)normals.size()) ? normals[ni] : glm::vec3(0.0f));
        mesh.uvs.push_back((ti >= 0 && ti < (int)uvs.size()) ? uvs[ti] : glm::vec2(0.0f));

        mesh.vertexMap[key] = newIdx;
        return newIdx;
        };

    while (std::getline(iss, line)) {
        std::istringstream ls(line);
        std::string prefix;
        ls >> prefix;
        if (prefix.empty() || prefix[0] == '#') continue;

        if (prefix == "v") {
            glm::vec3 v; ls >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (prefix == "vn") {
            glm::vec3 n; ls >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "vt") {
            glm::vec2 uv; ls >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (prefix == "o" || prefix == "g") {

            if (hasActiveGeometry) {
                startNewMesh(currentMaterial);
            }
        }
        else if (prefix == "usemtl") {
            ls >> currentMaterial;
            
            if (hasActiveGeometry && parsedMeshes.back().materialName != currentMaterial) {
                startNewMesh(currentMaterial);
            }
            else {
                parsedMeshes.back().materialName = currentMaterial;
            }
        }
        else if (prefix == "f") {
            hasActiveGeometry = true;
            struct OBJVertex { int vi = 0, ti = 0, ni = 0; };
            std::vector<OBJVertex> faceVertices;
            std::string vertexStr;

            while (ls >> vertexStr) {
                OBJVertex v;
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::istringstream vStream(vertexStr);

                if (vertexStr.find("  ") != std::string::npos) {
                    vStream >> v.vi >> v.ni;
                }
                else {
                    vStream >> v.vi;
                    if (vStream >> v.ti) vStream >> v.ni;
                }
                faceVertices.push_back(v);
            }

            if (faceVertices.size() >= 3) {
                TempMesh& currentMesh = parsedMeshes.back();
                for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
                    currentMesh.indices.push_back(addVertexToMesh(currentMesh, faceVertices[0].vi, faceVertices[0].ti, faceVertices[0].ni));
                    currentMesh.indices.push_back(addVertexToMesh(currentMesh, faceVertices[i + 1].vi, faceVertices[i + 1].ti, faceVertices[i + 1].ni));
                    currentMesh.indices.push_back(addVertexToMesh(currentMesh, faceVertices[i].vi, faceVertices[i].ti, faceVertices[i].ni));
                }
            }
        }
    }

    bool fileHasNormals = !normals.empty();
    struct GLVertex { glm::vec3 pos; glm::vec2 uv; glm::vec3 normal; glm::vec3 tangent; };

    for (auto& data : parsedMeshes) {
        if (data.indices.empty()) continue;

        if (!fileHasNormals) {
            data.normals.resize(data.vertices.size(), glm::vec3(0.0f));
            calculateNormals(data.vertices.data(), data.indices.data(), data.vertices.size(), data.indices.size(), data.normals.data());
        }

        std::vector<glm::vec3> tangents(data.vertices.size(), glm::vec3(0.0f));
        calculateTangents(data.vertices.data(), data.uvs.data(), data.normals.data(), data.indices.data(), data.vertices.size(), data.indices.size(), tangents.data());

        debugDumpMesh(data.materialName.c_str(), data.vertices.data(), data.vertices.size(), data.uvs.data(), data.normals.data(), tangents.data(), data.indices.data(), data.indices.size());

        std::vector<GLVertex> glBuffer(data.vertices.size());
        for (size_t i = 0; i < data.vertices.size(); ++i) {
            glBuffer[i].pos = data.vertices[i];
            glBuffer[i].uv = data.uvs[i];
            glBuffer[i].normal = data.normals[i];
            glBuffer[i].tangent = tangents[i];
        }

        VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();
        vbo->createIndirect(glBuffer.data(), (GLsizei)(glBuffer.size() * sizeof(GLVertex)),
            data.indices.data(), (GLsizei)(data.indices.size() * sizeof(uint32_t)),
            VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);

        Mesh* mesh = new Mesh();
        mesh->vbo = vbo;
        mesh->material = new Material();
        strncpy_s(mesh->material->name, sizeof(mesh->material->name), data.materialName.c_str(), _TRUNCATE);

        m_Meshes[m_nMeshCount++] = mesh;
        calculateBoundingBox(data.vertices.data(), data.vertices.size(), mesh->bboxMin, mesh->bboxMax);
    }

    return TRUE;
}


int Scene::parseOBJ2(uint8_t* buffer, size_t size) {
    std::istringstream iss(std::string(reinterpret_cast<char*>(buffer), size));
    std::string line;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::unordered_map<std::string, Material*> materialMap;
    Material* defaultMaterial = new Material();
    strncpy_s(defaultMaterial->name, sizeof(defaultMaterial->name), "default", _TRUNCATE);
    materialMap["default"] = defaultMaterial;

    while (std::getline(iss, line)) {
        std::istringstream ls(line);
        std::string prefix;
        ls >> prefix;
        if (prefix.empty() || prefix[0] == '#') continue;

        if (prefix == "v") {
            glm::vec3 v; ls >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (prefix == "vn") {
            glm::vec3 n; ls >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "vt") {
            glm::vec2 uv; ls >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
    }

    iss.clear();
    iss.seekg(0);

    glm::vec3* finalVertices = nullptr;
    glm::vec3* finalNormals = nullptr;
    glm::vec2* finalUVs = nullptr;
    uint32_t* indices = nullptr;
    size_t finalCount = 0, finalCap = 0;
    size_t indexCount = 0, indexCap = 0;
    bool objHasNormals = !normals.empty();

    Material* currentMaterial = defaultMaterial;
    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;

    auto addVertex = [&](int vi, int ti, int ni) -> uint32_t {
        VertexKey key{ vi, ti, ni };
        auto it = vertexMap.find(key);
        if (it != vertexMap.end()) return it->second;

        if (!finalVertices) {
            finalCap = 8;
            finalVertices = new glm::vec3[finalCap]();      // value-initialize to zero
            finalNormals = new glm::vec3[finalCap]();       // value-initialize to zero
            finalUVs = new glm::vec2[finalCap]();           // value-initialize to zero
        }
        if (finalCount >= finalCap) {
            finalCap *= 2;
            
            // Safe resize with explicit copy and cleanup
            glm::vec3* tmpV = new glm::vec3[finalCap]();
            if (finalVertices) {
                std::memcpy(tmpV, finalVertices, finalCount * sizeof(glm::vec3));
                delete[] finalVertices;
            }
            finalVertices = tmpV;
            
            glm::vec3* tmpN = new glm::vec3[finalCap]();
            if (finalNormals) {
                std::memcpy(tmpN, finalNormals, finalCount * sizeof(glm::vec3));
                delete[] finalNormals;
            }
            finalNormals = tmpN;
            
            glm::vec2* tmpUV = new glm::vec2[finalCap]();
            if (finalUVs) {
                std::memcpy(tmpUV, finalUVs, finalCount * sizeof(glm::vec2));
                delete[] finalUVs;
            }
            finalUVs = tmpUV;
        }

        finalVertices[finalCount] = (vi >= 0 && vi < (int)positions.size()) ? positions[vi] : glm::vec3(0.0f);
        finalNormals[finalCount] = (ni >= 0 && ni < (int)normals.size()) ? normals[ni] : glm::vec3(0.0f);
        finalUVs[finalCount] = (ti >= 0 && ti < (int)uvs.size()) ? uvs[ti] : glm::vec2(0.0f);

        vertexMap[key] = (uint32_t)finalCount;

        return (uint32_t)(finalCount++);
    };

    auto createMesh = [&]() -> Mesh* {
        if (finalCount == 0 || indexCount == 0) return nullptr;

        if (!objHasNormals)
            calculateNormals(finalVertices, indices, finalCount, indexCount, finalNormals);

        glm::vec3* tangents = new glm::vec3[finalCount]();  // Initialize to zero with ()
        calculateTangents(finalVertices, finalUVs, finalNormals, indices, finalCount, indexCount, tangents);

        // Debug dump before GPU upload
        debugDumpMesh(currentMaterial->name, finalVertices, finalCount, finalUVs, finalNormals, tangents, indices, indexCount);
        
        VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();

        vbo->createIndirect(finalVertices, (GLsizei)(finalCount * sizeof(glm::vec3)),
            indices, (GLsizei)(indexCount * sizeof(uint32_t)),
            VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);

        Mesh* mesh = new Mesh();
        mesh->vbo = vbo;
        mesh->material = currentMaterial;
        m_Meshes[m_nMeshCount++] = mesh;
        calculateBoundingBox(finalVertices, finalCount, mesh->bboxMin, mesh->bboxMax);

        delete[] tangents;
        delete[] finalVertices; finalVertices = nullptr; finalCount = 0; finalCap = 0;
        delete[] finalNormals; finalNormals = nullptr;
        delete[] finalUVs; finalUVs = nullptr;
        delete[] indices; indices = nullptr; indexCount = indexCap = 0;
        vertexMap.clear();
        objHasNormals = !normals.empty();

        return mesh;
    };

    auto findOrCreateMaterial = [&](const std::string& name) -> Material* {
        auto it = materialMap.find(name);
        if (it != materialMap.end()) return it->second;
        Material* m = new Material();
        strncpy_s(m->name, sizeof(m->name), name.c_str(), _TRUNCATE);
        return materialMap[name] = m;
    };

    bool parsingFaces = false;

    while (std::getline(iss, line)) {
         std::istringstream ls(line);
         std::string prefix;
         ls >> prefix;
         
         // Skip empty lines and pure comments
         if (prefix.empty() || (prefix[0] == '#' && prefix.length() == 1)) {
             continue;
         }
         
         // Statistics line (e.g., "#411 polygons") signals EOF for this mesh group
         if (prefix[0] == '#' && prefix.length() > 1 && std::isdigit(prefix[1])) {
             if (parsingFaces) {
                 createMesh();
                 parsingFaces = false;
             }
             continue;
         }

         // New group/object: finalize current mesh if one was being parsed
         if (prefix == "o" || prefix == "g") {
             if (parsingFaces) {
                 createMesh();
                 parsingFaces = false;
             }
             // Start collecting faces for this new group (material will be set by usemtl or default)
             parsingFaces = false; // will become true when first 'f' is encountered
             continue;
         }
         
         // Material change: finalize current mesh, switch material, prepare for new faces
         if (prefix == "usemtl") {
             if (parsingFaces) {
                 createMesh();
                 parsingFaces = false;
             }
             std::string matName;
             ls >> matName;
             currentMaterial = findOrCreateMaterial(matName);
             continue;
         }
         
         // Parse face: accumulate vertices/triangles into current mesh
         if (prefix == "f") {
             parsingFaces = true;

             struct OBJVertex { int vi = 0, ti = 0, ni = 0; };
             std::vector<OBJVertex> faceVertices;
             std::string vertexStr;

             while (ls >> vertexStr) {
                 OBJVertex v;
                 std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                 std::istringstream vStream(vertexStr);

                 if (vertexStr.find("  ") != std::string::npos) {
                     vStream >> v.vi >> v.ni;
                 }
                 else {
                      vStream >> v.vi;
                     if (vStream >> v.ti) {
                         vStream >> v.ni;
                     }
                 }

                 if (v.vi > 0) v.vi--; else if (v.vi < 0) v.vi = (int)positions.size() + v.vi;
                 if (v.ti > 0) v.ti--; else if (v.ti < 0) v.ti = (int)uvs.size() + v.ti;
                 if (v.ni > 0) v.ni--; else if (v.ni < 0) v.ni = (int)normals.size() + v.ni;

                 faceVertices.push_back(v);
             }

             if (faceVertices.size() >= 3) {
                 for (size_t i = 1; i < faceVertices.size() - 1; ++i) {

                     if (!indices) {
                         indexCap = 16;
                         indices = new uint32_t[indexCap]();
                     }
                     if (indexCount + 3 > indexCap) {
                         indexCap *= 2;
                         uint32_t* tmpIdx = new uint32_t[indexCap]();
                         if (indices) {
                             std::memcpy(tmpIdx, indices, indexCount * sizeof(uint32_t));
                             delete[] indices;
                         }
                         indices = tmpIdx;
                     }

                     indices[indexCount++] = addVertex(faceVertices[0].vi, faceVertices[0].ti, faceVertices[0].ni);
                     indices[indexCount++] = addVertex(faceVertices[i].vi, faceVertices[i].ti, faceVertices[i].ni);
                     indices[indexCount++] = addVertex(faceVertices[i + 1].vi, faceVertices[i + 1].ti, faceVertices[i + 1].ni);
                 }
             }
             continue;
         }
     }
     
     // EOF: finalize any remaining mesh
     if (parsingFaces) {
         createMesh();
     }
     
     return TRUE;
}

void Scene::calculateNormals(glm::vec3* vertices, uint32_t* indices,
    size_t vertexCount, size_t indexCount, glm::vec3* normals) {

    for (size_t i = 0; i < vertexCount; ++i)
        normals[i] = glm::vec3(0.0f);

    for (size_t i = 0; i + 2 < indexCount; i += 3) {
        uint32_t i0 = indices[i + 0];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        const glm::vec3& v0 = vertices[i0];
        const glm::vec3& v1 = vertices[i1];
        const glm::vec3& v2 = vertices[i2];

        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;

        glm::vec3 faceNormal = glm::cross(e1, e2);

        float len2 = glm::dot(faceNormal, faceNormal);
        if (len2 < 1e-12f)
            continue; 

        normals[i0] += faceNormal;
        normals[i1] += faceNormal;
        normals[i2] += faceNormal;
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        float len2 = glm::dot(normals[i], normals[i]);
        if (len2 > 0.0f)
            normals[i] = glm::normalize(normals[i]);
        else
            normals[i] = glm::vec3(0.0f, 1.0f, 0.0f); 
    }
}

void Scene::calculateTangents(glm::vec3* vertices, glm::vec2* uvs, glm::vec3* normals,
    uint32_t* indices, size_t vertexCount, size_t indexCount,
    glm::vec3* tangents) {
    for (size_t i = 0; i < vertexCount; ++i) tangents[i] = glm::vec3(0.0f);

    for (size_t i = 0; i < indexCount; i += 3) {
        uint32_t i0 = indices[i + 0];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        glm::vec3& v0 = vertices[i0];
        glm::vec3& v1 = vertices[i1];
        glm::vec3& v2 = vertices[i2];

        glm::vec2& uv0 = uvs[i0];
        glm::vec2& uv1 = uvs[i1];
        glm::vec2& uv2 = uvs[i2];

        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float r = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (fabs(r) < 1e-8f) r = 1.0f;

        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

        tangents[i0] += tangent;
        tangents[i1] += tangent;
        tangents[i2] += tangent;
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        tangents[i] = glm::normalize(tangents[i]);
    }
}

void Scene::calculateBoundingBox(glm::vec3* vertices, size_t vertexCount,
    glm::vec3& outMin, glm::vec3& outMax) {
    if (vertexCount == 0) return;
    outMin = vertices[0];
    outMax = vertices[0];

    for (size_t i = 1; i < vertexCount; ++i) {
        outMin = glm::min(outMin, vertices[i]);
        outMax = glm::max(outMax, vertices[i]);
    }
}

void Scene::debugDumpMesh(const char* materialName, glm::vec3* vertices, size_t vertexCount,
                          glm::vec2* uvs, glm::vec3* normals, glm::vec3* tangents,
                          uint32_t* indices, size_t indexCount) {
    std::ofstream debugFile("mesh_debug.txt", std::ios::app);
    if (!debugFile.is_open()) return;

    debugFile << "=== MESH DEBUG: Material='" << materialName << "' ===\n";
    debugFile << "Vertex Count: " << vertexCount << ", Index Count: " << indexCount 
              << ", Triangle Count: " << (indexCount / 3) << "\n\n";

    // ALL vertices
    debugFile << "--- ALL " << vertexCount << " Vertices ---\n";
    for (size_t i = 0; i < vertexCount; ++i) {
        debugFile << "[" << i << "] Pos:(" 
                  << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << ") "
                  << "UV:(" << uvs[i].x << ", " << uvs[i].y << ") "
                  << "Normal:(" << normals[i].x << ", " << normals[i].y << ", " << normals[i].z << ") "
                  << "Tangent:(" << tangents[i].x << ", " << tangents[i].y << ", " << tangents[i].z << ")\n";
    }

    // ALL triangles
    debugFile << "\n--- ALL " << (indexCount / 3) << " Triangles ---\n";
    for (size_t tri = 0; tri < indexCount / 3; ++tri) {
        uint32_t idx0 = indices[tri * 3 + 0];
        uint32_t idx1 = indices[tri * 3 + 1];
        uint32_t idx2 = indices[tri * 3 + 2];
        debugFile << "Triangle[" << tri << "]: idx(" << idx0 << ", " << idx1 << ", " << idx2 << ")\n";

        if (idx0 < vertexCount && idx1 < vertexCount && idx2 < vertexCount) {
            debugFile << "  V0: (" << vertices[idx0].x << ", " << vertices[idx0].y << ", " << vertices[idx0].z << ")\n";
            debugFile << "  V1: (" << vertices[idx1].x << ", " << vertices[idx1].y << ", " << vertices[idx1].z << ")\n";
            debugFile << "  V2: (" << vertices[idx2].x << ", " << vertices[idx2].y << ", " << vertices[idx2].z << ")\n";
        }
    }

    // Bounding box
    glm::vec3 meshMin = vertices[0];
    glm::vec3 meshMax = vertices[0];
    for (size_t i = 1; i < vertexCount; ++i) {
        meshMin = glm::min(meshMin, vertices[i]);
        meshMax = glm::max(meshMax, vertices[i]);
    }
    debugFile << "\nBBox Min: (" << meshMin.x << ", " << meshMin.y << ", " << meshMin.z << ")\n";
    debugFile << "BBox Max: (" << meshMax.x << ", " << meshMax.y << ", " << meshMax.z << ")\n";
    debugFile << "=== END MESH DEBUG ===\n\n";

    debugFile.close();
}