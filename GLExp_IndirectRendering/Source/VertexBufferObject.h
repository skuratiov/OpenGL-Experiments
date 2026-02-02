#pragma once


enum VERTEX_DATA_FORMAT {
	FLOAT_VX2,
	FLOAT_VX3,
	FLOAT_VX3UV2,
	FLOAT_VX3CL4,
	FLOAT_VX3UV2NR3,
	FLOAT_VX3UV2NR3TN3
};

class VertexBufferObject {
protected:
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

		if (m_hasIndex) {
			glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBufferObject);
			glDrawElements(GL_TRIANGLES, m_nIndexCount, GL_UNSIGNED_INT, 0); }
		else { 
			glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
			glDrawArrays(GL_TRIANGLES, 0, m_nVertexCount); 
		}
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

