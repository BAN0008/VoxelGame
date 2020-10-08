#include "VertexBuffer.hpp"
#include "Log.hpp"
#include <glad/glad.h>
#include <string>

int GetAttributeSize(const uint32_t type)
{
	switch ((type & 070) >> 3) {
		case 0:  return 2 * (type & 007); // Float16
		case 1:  return 4 * (type & 007); // Float32
		case 2:  return 1 * (type & 007); // Int8
		case 3:  return 1 * (type & 007); // Uint8
		case 4:  return 2 * (type & 007); // Int16
		case 5:  return 2 * (type & 007); // Uint16
		case 6:  return 4 * (type & 007); // Int32
		case 7:  return 4 * (type & 007); // Uint32
		default: return 0;
	}
}

GLenum GetAttributeType(const uint32_t type)
{
	switch ((type & 070) >> 3) {
		case 0:  return GL_HALF_FLOAT;     // Float16
		case 1:  return GL_FLOAT;          // Float32
		case 2:  return GL_BYTE;           // Int8
		case 3:  return GL_UNSIGNED_BYTE;  // Uint8
		case 4:  return GL_SHORT;          // Int16
		case 5:  return GL_UNSIGNED_SHORT; // Uint16
		case 6:  return GL_INT;            // Int32
		case 7:  return GL_UNSIGNED_INT;   // Uint32
		default: return 0;
	}
}

GLint GetAttributeComponentCount(const uint32_t type)
{
	return type & 007;
}

VertexBuffer::VertexBuffer(std::initializer_list<VertexType> attributes, const char *name) : name(name)
{
	// Calculate stride
	stride = 0;
	for (const VertexType attribute : attributes) {
		stride += GetAttributeSize(static_cast<uint32_t>(attribute));
	}

	// Create vertex array object
	#ifdef ARB_DIRECT_STATE_ACCESS
		glCreateVertexArrays(1, &vao);
	#else
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	#endif
	if (GLAD_GL_KHR_debug && !this->name.empty()) glObjectLabel(GL_VERTEX_ARRAY, vao, -1, (std::string(name) + " (VAO)").c_str());

	// Create vertex buffer object
	#ifdef ARB_DIRECT_STATE_ACCESS
		glCreateBuffers(1, &vbo);
	#else
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	#endif
	if (GLAD_GL_KHR_debug && !this->name.empty()) glObjectLabel(GL_BUFFER, vbo, -1, (std::string(name) + " (VBO)").c_str());
	#ifdef ARB_DIRECT_STATE_ACCESS
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, stride);
	#endif

	int index  = 0;
	int offset = 0;
	for (const VertexType attribute : attributes) {
		#ifdef ARB_DIRECT_STATE_ACCESS
			glEnableVertexArrayAttrib(vao, index);
			glVertexArrayAttribBinding(vao, index, 0);
			glVertexArrayAttribFormat(vao, index, GetAttributeComponentCount(static_cast<uint32_t>(attribute)), GetAttributeType(static_cast<uint32_t>(attribute)), GL_FALSE, offset);
			// glVertexArrayAttribIFormat(vao, index, GetAttributeComponentCount(static_cast<uint32_t>(attribute)), GetAttributeType(static_cast<uint32_t>(attribute)), offset);
		#else
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, GetAttributeComponentCount(static_cast<uint32_t>(attribute)), GetAttributeType(static_cast<uint32_t>(attribute)), GL_FALSE, stride, reinterpret_cast<void*>(offset));
		#endif
		// Log::Info(std::string("Configured Attribute: index ") + std::to_string(index) + " comp " + std::to_string(GetAttributeComponentCount(static_cast<uint32_t>(attribute))) + " type " + std::to_string(GetAttributeType(static_cast<uint32_t>(attribute))) + " offset " + std::to_string(offset));
		offset += GetAttributeSize(static_cast<uint32_t>(attribute));
		index++;
	}
	#ifndef ARB_DIRECT_STATE_ACCESS
		glBindVertexArray(0);
	#endif
}

VertexBuffer::~VertexBuffer()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void VertexBuffer::UpdateVertices(const void *vertices, uint64_t vertexCount)
{
	if (vertexCount == 0) return;
	this->vertexCount = vertexCount;
	#ifdef ARB_DIRECT_STATE_ACCESS
		if (vertexCount * stride > vboSize) {
			vboSize = vertexCount * stride;
			glNamedBufferData(vbo, vboSize, vertices, GL_DYNAMIC_DRAW);
		}
		else {
			glNamedBufferSubData(vbo, 0, vertexCount * stride, vertices);
		}
	#else
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		if (vertexCount * stride > vboSize) {
			vboSize = vertexCount * stride;
			glBufferData(GL_ARRAY_BUFFER, vboSize, vertices, GL_DYNAMIC_DRAW);
		}
		else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * stride, vertices);
		}
	#endif
}

void VertexBuffer::UpdateIndices(const uint32_t *indices, uint64_t indexCount)
{
	if (indexCount == 0) return;
	this->indexCount = indexCount;
	if (ebo == 0) {
		// Create element buffer object
		#ifdef ARB_DIRECT_STATE_ACCESS
			glCreateBuffers(1, &ebo);
		#else
			glBindVertexArray(vao);
			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		#endif
		if (GLAD_GL_KHR_debug && !name.empty()) glObjectLabel(GL_BUFFER, ebo, -1, (name + " (EBO)").c_str());
		#ifdef ARB_DIRECT_STATE_ACCESS
			glVertexArrayElementBuffer(vao, ebo);
		#endif
	}
	#ifdef ARB_DIRECT_STATE_ACCESS
		if (indexCount * 4 > eboSize) {
			eboSize = indexCount * 4;
			glNamedBufferData(ebo, eboSize, indices, GL_DYNAMIC_DRAW);
		}
		else {
			glNamedBufferSubData(ebo, 0, indexCount * 4, indices);
		}
	#else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		if (indexCount * 4 > eboSize) {
			eboSize = indexCount * 4;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * 4, indices, GL_DYNAMIC_DRAW);
		}
		else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * 4, indices);	
		}
	#endif
}

void VertexBuffer::Render(uint64_t vertexCount)
{
	vertexCount = vertexCount == 0 ? this->vertexCount : vertexCount;
	glBindVertexArray(vao);
	if (ebo == 0) {
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}
	else {
		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
}