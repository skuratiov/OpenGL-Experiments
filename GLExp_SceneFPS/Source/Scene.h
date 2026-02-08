#pragma once

#define MAX_MESHES	256

struct Material {
	glm::vec3 diffuseColor = glm::vec3(1.0f);   
	glm::vec3 specularColor = glm::vec3(1.0f);  
	float shininess = 32.0f;                    
	
	Image* diffuseTex = nullptr;  
	Image* normalMap = nullptr;      
};

struct Mesh {
	VertexBufferObjectIndirect* vbo = nullptr;
	Material* material = nullptr;
};

class Scene {
public:
	static Scene* getInstance() {
		return (!m_pInstance) ?
			m_pInstance = new Scene() : m_pInstance;
	}

	Scene(const Scene&) = delete;
	virtual ~Scene();

	BOOL fromOBJ(const char*);

protected:
	Scene();

private:
	static Scene* m_pInstance;

	Mesh* m_Meshes[MAX_MESHES];
	size_t m_nMeshCount = 0;

	BOOL parseOBJ(uint8_t* buffer, size_t size);

	void calculateTangents(glm::vec3* vertices, glm::vec2* uvs, glm::vec3* normals,
		uint32_t* indices, size_t vertexCount, size_t indexCount,
		glm::vec3* tangents);
};

