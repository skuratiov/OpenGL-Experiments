#pragma once


enum VERTEX_DATA_FORMAT {
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
	bool m_hasIndex;

public:
	VertexBufferObject();
	~VertexBufferObject();

	void createBuffers(void *, GLsizei, void*, GLsizei, uint16_t );
	void deleteBuffers();

	inline void draw() const {
		glBindVertexArray(m_nVertexArrayId);

		if (m_hasIndex) { glDrawElements(GL_TRIANGLES, m_nIndexCount, GL_UNSIGNED_INT, 0); }
		else { glDrawArrays(GL_TRIANGLES, 0, m_nVertexCount); }
	}

	inline void unbind() { glBindVertexArray(0); }
};

