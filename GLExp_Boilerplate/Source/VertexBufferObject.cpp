#include "framework.h"
#include "VertexBufferObject.h"

//
// Constructor / destructor
//
VertexBufferObject::VertexBufferObject() {
	m_nVertexArrayId = m_nVertexBufferObject = m_nIndexBufferObject = 0;
	m_nVertexCount = m_nIndexCount = 0;
	m_hasIndex = false;
}

VertexBufferObject::~VertexBufferObject() {
	deleteBuffers();
}

void VertexBufferObject::createBuffers(void *vertexData, GLsizei vertexDataSize,
	void *indexData, GLsizei indexDataSize, uint16_t format) {
	glGenVertexArrays(1, &m_nVertexArrayId);
	glGenBuffers(1, &m_nVertexBufferObject);

	glBindVertexArray(m_nVertexArrayId);

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);

	GLsizei stride = 0;
	m_hasIndex = false;
	m_nIndexCount = 0;

	switch (format) {
	case VERTEX_DATA_FORMAT::FLOAT_VX3:
		stride = 3 * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		break;
	case VERTEX_DATA_FORMAT::FLOAT_VX3UV2:
		stride = 5 * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		break;
	case VERTEX_DATA_FORMAT::FLOAT_VX3CL4:
		stride = 7 * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		break;
	case VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3:
		stride = 8 * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
		break;
	case VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3:
		stride = 11 * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
		break;
	}

	if (indexData) {
		m_hasIndex = true;

		glGenBuffers(1, &m_nIndexBufferObject);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBufferObject);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indexDataSize,
			indexData,
			GL_STATIC_DRAW
		);

		m_nIndexCount = (GLsizei) (indexDataSize / sizeof(uint32_t));
	}

	m_nVertexCount = (GLsizei) (vertexDataSize / stride);
	
	glBindVertexArray(0);
}

void VertexBufferObject::deleteBuffers() {
	
	glBindVertexArray(0);

	if (m_nVertexBufferObject) {
		glDeleteBuffers(1, &m_nVertexBufferObject);
		m_nVertexBufferObject = 0;
	}
	
	if (m_nIndexBufferObject) {
		glDeleteBuffers(1, &m_nIndexBufferObject);
		m_nIndexBufferObject = 0;
	}

	if (m_nVertexArrayId) {
		glDeleteVertexArrays(1, &m_nVertexArrayId);
		m_nVertexArrayId = 0;
	}

	m_nVertexCount = 0;
	m_nIndexCount = 0;
	m_hasIndex = false;
}