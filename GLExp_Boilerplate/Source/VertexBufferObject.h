#pragma once


enum VERTEX_DATA_FORMAT {
	FLOAT_VX3,
	FLOAT_VX3UV2,
	FLOAT_VX3CL4,
	FLOAT_VX3UV2NR3
};

class VertexBufferObject {
private:
	GLuint m_nVertexArrayId,
		   m_nVertexBufferObject,
		   m_nIndexBufferObject;

	size_t m_nVertexCount, m_nIndexCount;
	bool m_hasIndex;

	inline void draw() const {
		glBindVertexArray(m_nVertexArrayId);

		if (m_hasIndex)
			glDrawElements(GL_TRIANGLES, m_nIndexCount, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, m_nVertexCount);
	}

public:
	VertexBufferObject();
	~VertexBufferObject();

	void createBuffers(void *, size_t , void*, size_t, uint16_t );
	void deleteBuffers();
};

