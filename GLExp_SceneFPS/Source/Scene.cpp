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
            finalVertices = new glm::vec3[finalCap];
            finalNormals = new glm::vec3[finalCap];
            finalUVs = new glm::vec2[finalCap];
        }
        if (finalCount >= finalCap) {
            finalCap *= 2;
            glm::vec3* tmpV = new glm::vec3[finalCap]; memcpy(tmpV, finalVertices, finalCount * sizeof(glm::vec3)); delete[] finalVertices; finalVertices = tmpV;
            glm::vec3* tmpN = new glm::vec3[finalCap]; memcpy(tmpN, finalNormals, finalCount * sizeof(glm::vec3)); delete[] finalNormals; finalNormals = tmpN;
            glm::vec2* tmpUV = new glm::vec2[finalCap]; memcpy(tmpUV, finalUVs, finalCount * sizeof(glm::vec2)); delete[] finalUVs; finalUVs = tmpUV;
        }

        finalVertices[finalCount] = (vi >= 0 && vi < positions.size()) ? positions[vi] : glm::vec3(0.0f);
        finalNormals[finalCount] = (ni >= 0 && ni < normals.size()) ? normals[ni] : glm::vec3(0.0f);
        finalUVs[finalCount] = (ti >= 0 && ti < uvs.size()) ? uvs[ti] : glm::vec2(0.0f);

        vertexMap[key] = finalCount;

        return finalCount++;
    };

    auto createMesh = [&]() -> Mesh* {
        if (finalCount == 0 || indexCount == 0) return nullptr;

        if (!objHasNormals)
            calculateNormals(finalVertices, indices, finalCount, indexCount, finalNormals);

        glm::vec3* tangents = new glm::vec3[finalCount];
        calculateTangents(finalVertices, finalUVs, finalNormals, indices, finalCount, indexCount, tangents);

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
        if (prefix.empty() || prefix[0] == '#') continue;

        if (prefix == "o" || prefix == "g") {
            if (parsingFaces) createMesh();
            parsingFaces = true;
            currentMaterial = defaultMaterial;
        }
        else if (prefix == "usemtl") {
            std::string matName; ls >> matName;
            currentMaterial = findOrCreateMaterial(matName);
        }
        else if (prefix == "f") {
            parsingFaces = true;
            std::vector<std::tuple<int, int, int>> polygonVerts;

            std::string vert;
            while (ls >> vert) {
                int vi = -1, ti = -1, ni = -1;
                size_t first = vert.find('/'), second = vert.rfind('/');

                vi = std::stoi(vert.substr(0, first)) - 1;
                if (first != std::string::npos && second > first) {
                    std::string mid = vert.substr(first + 1, second - first - 1);
                    if (!mid.empty()) ti = std::stoi(mid) - 1;
                    ni = std::stoi(vert.substr(second + 1)) - 1;
                }

                polygonVerts.push_back(std::make_tuple(vi, ti, ni));
            }

            for (size_t j = 1; j + 1 < polygonVerts.size(); ++j) {
                int vi0 = std::get<0>(polygonVerts[0]);
                int ti0 = std::get<1>(polygonVerts[0]);
                int ni0 = std::get<2>(polygonVerts[0]);
                int vi1 = std::get<0>(polygonVerts[j]);
                int ti1 = std::get<1>(polygonVerts[j]);
                int ni1 = std::get<2>(polygonVerts[j]);
                int vi2 = std::get<0>(polygonVerts[j + 1]);
                int ti2 = std::get<1>(polygonVerts[j + 1]);
                int ni2 = std::get<2>(polygonVerts[j + 1]);

                uint32_t idx0 = addVertex(vi0, ti0, ni0);
                uint32_t idx1 = addVertex(vi1, ti1, ni1);
                uint32_t idx2 = addVertex(vi2, ti2, ni2);

                if (indexCount + 3 > indexCap) {
                    indexCap = std::max(indexCap ? indexCap * 2 : 8, indexCount + 3);
                    uint32_t* tmp = new uint32_t[indexCap];
                    if (indices) { memcpy(tmp, indices, indexCount * sizeof(uint32_t)); delete[] indices; }
                    indices = tmp;
                }

                indices[indexCount++] = idx0;
                indices[indexCount++] = idx1;
                indices[indexCount++] = idx2;
            }
        }
    }

    if (parsingFaces) createMesh();

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