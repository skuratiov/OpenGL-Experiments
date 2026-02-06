#pragma once


enum VERTEX_DATA_FORMAT {
	FLOAT_VX2,
	FLOAT_VX2UV2,
	FLOAT_VX3,
	FLOAT_VX3UV2,
	FLOAT_VX3CL4,
	FLOAT_VX3UV2NR3,
	FLOAT_VX3UV2NR3TN3
};

class VertexBufferObject {
private:
	GLuint m_nVertexArrayId,
		   m_nVertexBufferObject,
		   m_nIndexBufferObject;

	GLsizei m_nVertexCount, m_nIndexCount;
	bool m_hasIndex, m_isDynamic;

public:
	VertexBufferObject();
	~VertexBufferObject();

	void createBuffers(void *, GLsizei, void*, GLsizei, uint16_t, bool isDynamic = false);
	void deleteBuffers();

	inline void draw() const {
		glBindVertexArray(m_nVertexArrayId);

		if (m_hasIndex) {
			glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBufferObject);
			glDrawElements(GL_TRIANGLES, m_nIndexCount, GL_UNSIGNED_INT, 0); }
		else { 
			glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
			glDrawArrays(GL_TRIANGLES, 0, m_nVertexCount); 
		}
	}

	inline void update(const void* data, size_t amount, GLsizei size) {
		if (!m_isDynamic) return; 

		m_nVertexCount = amount;

		glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * amount, data);
	}

	inline void bind() {
		glBindVertexArray(m_nVertexArrayId);
		if (m_hasIndex) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBufferObject); }
		glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
	}

	inline void unbind() {
		if (m_hasIndex) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

