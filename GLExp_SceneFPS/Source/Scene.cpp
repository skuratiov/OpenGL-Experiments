#include "framework.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
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
        m_Meshes[i]->material = nullptr;
        m_Meshes[i]->material = nullptr;
    }
	m_nMeshCount = 0;
}

Scene::~Scene() {
	for (size_t i = 0; i < m_nMeshCount; ++i) {
        if (m_Meshes[i]->vbo)
            m_Meshes[i]->vbo->deleteIndirect();
        delete m_Meshes[i];
		m_Meshes[i] = nullptr;
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

    glm::vec3* positions = nullptr; size_t posCount = 0, posCap = 0;
    glm::vec3* normals = nullptr; size_t normalCount = 0, normalCap = 0;
    glm::vec2* uvs = nullptr; size_t uvCount = 0, uvCap = 0;

    glm::vec3* finalVertices = nullptr; size_t finalCount = 0, finalCap = 0;
    glm::vec3* finalNormals = nullptr;
    glm::vec2* finalUVs = nullptr;
    glm::vec3* tangents = nullptr;
    uint32_t* indices = nullptr; size_t indexCount = 0, indexCap = 0;

    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;

    auto addVertex = [&](int vi, int ti, int ni) -> uint32_t {
        VertexKey key{ vi, ti, ni };
        auto it = vertexMap.find(key);
        if (it != vertexMap.end()) return it->second;

        uint32_t idx = (uint32_t)finalCount;

        if (finalCount >= finalCap) {
            finalCap = (finalCap == 0) ? 8 : finalCap * 2;
            glm::vec3* tmpV = new glm::vec3[finalCap]; if (finalVertices) { memcpy(tmpV, finalVertices, finalCount * sizeof(glm::vec3)); delete[] finalVertices; } finalVertices = tmpV;
            glm::vec3* tmpN = new glm::vec3[finalCap]; if (finalNormals) { memcpy(tmpN, finalNormals, finalCount * sizeof(glm::vec3)); delete[] finalNormals; } finalNormals = tmpN;
            glm::vec2* tmpUV = new glm::vec2[finalCap]; if (finalUVs) { memcpy(tmpUV, finalUVs, finalCount * sizeof(glm::vec2)); delete[] finalUVs; } finalUVs = tmpUV;
        }

        finalVertices[finalCount] = positions[vi];
        finalNormals[finalCount] = (ni >= 0) ? normals[ni] : glm::vec3(0.0f);
        finalUVs[finalCount] = (ti >= 0) ? uvs[ti] : glm::vec2(0.0f, 0.0f);

        vertexMap[key] = idx;
        finalCount++;
        return idx;
    };

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string prefix;
        lineStream >> prefix;

        if (prefix == "v") {
            glm::vec3 v; lineStream >> v.x >> v.y >> v.z;
            if (posCount >= posCap) { posCap = (posCap == 0) ? 8 : posCap * 2; glm::vec3* tmp = new glm::vec3[posCap]; if (positions) { memcpy(tmp, positions, posCount * sizeof(glm::vec3)); delete[] positions; } positions = tmp; }
            positions[posCount++] = v;
        }
        else if (prefix == "vn") {
            glm::vec3 n; lineStream >> n.x >> n.y >> n.z;
            if (normalCount >= normalCap) { normalCap = (normalCap == 0) ? 8 : normalCap * 2; glm::vec3* tmp = new glm::vec3[normalCap]; if (normals) { memcpy(tmp, normals, normalCount * sizeof(glm::vec3)); delete[] normals; } normals = tmp; }
            normals[normalCount++] = n;
        }
        else if (prefix == "vt") {
            glm::vec2 uv; lineStream >> uv.x >> uv.y;
            if (uvCount >= uvCap) { uvCap = (uvCap == 0) ? 4 : uvCap * 2; glm::vec2* tmp = new glm::vec2[uvCap]; if (uvs) { memcpy(tmp, uvs, uvCount * sizeof(glm::vec2)); delete[] uvs; } uvs = tmp; }
            uvs[uvCount++] = uv;
        }
        else if (prefix == "f") {
            std::string vert;
            while (lineStream >> vert) {
                int vi = -1, ti = -1, ni = -1;
                size_t first = vert.find('/');
                size_t second = vert.rfind('/');
                vi = std::stoi(vert.substr(0, first)) - 1;
                if (first != std::string::npos && second > first) {
                    std::string mid = vert.substr(first + 1, second - first - 1);
                    if (!mid.empty()) ti = std::stoi(mid) - 1;
                    ni = std::stoi(vert.substr(second + 1)) - 1;
                }

                uint32_t idx = addVertex(vi, ti, ni);
                if (indexCount >= indexCap) { indexCap = (indexCap == 0) ? 3 : indexCap * 2; uint32_t* tmp = new uint32_t[indexCap]; if (indices) { memcpy(tmp, indices, indexCount * sizeof(uint32_t)); delete[] indices; } indices = tmp; }
                indices[indexCount++] = idx;
            }
        }
        else if (prefix == "o" || prefix == "g" || prefix == "usemtl") {
            if (finalCount > 0 && indexCount > 0) {
                if (m_nMeshCount >= MAX_MESHES) { delete[] finalVertices; delete[] finalNormals; delete[] finalUVs; delete[] indices; return -40; }
                tangents = new glm::vec3[finalCount];
                calculateTangents(finalVertices, finalUVs, finalNormals, indices, finalCount, indexCount, tangents);

                VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();
                vbo->createIndirect(finalVertices, finalCount * sizeof(glm::vec3), indices, indexCount * sizeof(uint32_t), VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);
                m_Meshes[m_nMeshCount++]->vbo = vbo;

                delete[] finalVertices; finalVertices = nullptr; finalCount = 0; finalCap = 0;
                delete[] finalNormals; finalNormals = nullptr;
                delete[] finalUVs; finalUVs = nullptr;
                delete[] indices; indices = nullptr; indexCount = indexCap = 0;
                delete[] tangents; tangents = nullptr;
                vertexMap.clear();
            }
        }
    }

    if (finalCount > 0 && indexCount > 0) {
        if (m_nMeshCount >= MAX_MESHES) { delete[] finalVertices; delete[] finalNormals; delete[] finalUVs; delete[] indices; return -50; }
        tangents = new glm::vec3[finalCount];
        calculateTangents(finalVertices, finalUVs, finalNormals, indices, finalCount, indexCount, tangents);

        VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();
        vbo->createIndirect(finalVertices,(GLsizei) (finalCount * sizeof(glm::vec3)), 
            indices, indexCount * sizeof(uint32_t), VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);
        m_Meshes[m_nMeshCount++]->vbo = vbo;

        delete[] finalVertices; delete[] finalNormals; delete[] finalUVs; delete[] indices; delete[] tangents;
    }

    delete[] positions; delete[] normals; delete[] uvs;

    return TRUE;
}


/*
int Scene::parseOBJ(uint8_t* buffer, size_t size) {
    std::istringstream iss(std::string(reinterpret_cast<char*>(buffer), size));
    std::string line;

    glm::vec3* vertices = nullptr; size_t vertexCount = 0, vertexCap = 0;
    glm::vec3* normals = nullptr; size_t normalCount = 0, normalCap = 0;
    glm::vec2* uvs = nullptr; size_t uvCount = 0, uvCap = 0;
    uint32_t* indices = nullptr; size_t indexCount = 0, indexCap = 0;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string prefix;
        lineStream >> prefix;

        if (prefix == "o" || prefix == "g" || prefix == "usemtl") {
            if (vertexCount > 0 && indexCount > 0) {
                if (m_nMeshCount >= MAX_MESHES) {
                    delete[] vertices; delete[] normals; delete[] uvs; delete[] indices;
                    return -20; 
                }

                glm::vec3* tangents = new glm::vec3[vertexCount];
                calculateTangents(vertices, uvs, normals, indices, vertexCount, indexCount, tangents);

                VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();
                vbo->createIndirect(vertices, static_cast<GLsizei>(vertexCount * sizeof(glm::vec3)),
                    indices, static_cast<GLsizei>(indexCount * sizeof(uint32_t)),
                    VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);

                m_Meshes[m_nMeshCount++] = vbo;
                delete[] tangents;

                delete[] vertices; vertices = nullptr; vertexCount = vertexCap = 0;
                delete[] normals; normals = nullptr; normalCount = normalCap = 0;
                delete[] uvs; uvs = nullptr; uvCount = uvCap = 0;
                delete[] indices; indices = nullptr; indexCount = indexCap = 0;
            }
            continue;
        }

        if (prefix == "v") {
            glm::vec3 v; lineStream >> v.x >> v.y >> v.z;
            if (vertexCount >= vertexCap) {
                vertexCap = (vertexCap == 0) ? 8 : vertexCap * 2;
                glm::vec3* tmp = new glm::vec3[vertexCap];
                if (vertices) { memcpy(tmp, vertices, vertexCount * sizeof(glm::vec3)); delete[] vertices; }
                vertices = tmp;
            }
            vertices[vertexCount++] = v;
        }
        else if (prefix == "vn") {
            glm::vec3 n; lineStream >> n.x >> n.y >> n.z;
            if (normalCount >= normalCap) {
                normalCap = (normalCap == 0) ? 8 : normalCap * 2;
                glm::vec3* tmp = new glm::vec3[normalCap];
                if (normals) { memcpy(tmp, normals, normalCount * sizeof(glm::vec3)); delete[] normals; }
                normals = tmp;
            }
            normals[normalCount++] = n;
        }
        else if (prefix == "vt") {
            glm::vec2 uv; lineStream >> uv.x >> uv.y;
            if (uvCount >= uvCap) {
                uvCap = (uvCap == 0) ? 4 : uvCap * 2;
                glm::vec2* tmp = new glm::vec2[uvCap];
                if (uvs) { memcpy(tmp, uvs, uvCount * sizeof(glm::vec2)); delete[] uvs; }
                uvs = tmp;
            }
            uvs[uvCount++] = uv;
        }
        else if (prefix == "f") {
            std::string vert;
            while (lineStream >> vert) {
                size_t pos1 = vert.find('/');
                unsigned int vi = std::stoi(vert.substr(0, pos1)) - 1;
                if (indexCount >= indexCap) {
                    indexCap = (indexCap == 0) ? 3 : indexCap * 2;
                    uint32_t* tmp = new uint32_t[indexCap];
                    if (indices) { memcpy(tmp, indices, indexCount * sizeof(uint32_t)); delete[] indices; }
                    indices = tmp;
                }
                indices[indexCount++] = vi;
            }
        }
    }

    if (vertexCount > 0 && indexCount > 0) {
        if (m_nMeshCount >= MAX_MESHES) {
            delete[] vertices; delete[] normals; delete[] uvs; delete[] indices;
            return -30;
        }

        glm::vec3* tangents = new glm::vec3[vertexCount];
        calculateTangents(vertices, uvs, normals, indices, vertexCount, indexCount, tangents);

        VertexBufferObjectIndirect* vbo = new VertexBufferObjectIndirect();
        vbo->createIndirect(vertices, static_cast<GLsizei>(vertexCount * sizeof(glm::vec3)),
            indices, static_cast<GLsizei>(indexCount * sizeof(uint32_t)),
            VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);

        m_Meshes[m_nMeshCount++] = vbo;
        delete[] tangents;
    }

    delete[] vertices; delete[] normals; delete[] uvs; delete[] indices;

    return TRUE;
} */


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

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

        tangents[i0] += tangent;
        tangents[i1] += tangent;
        tangents[i2] += tangent;
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        tangents[i] = glm::normalize(tangents[i]);
    }
}