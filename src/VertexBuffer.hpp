#pragma once
#include <cstdint>
#include <initializer_list>
#include <string>

enum struct VertexType
{
	Float16   = 001,
	Float16_2 = 002,
	Float16_3 = 003,
	Float16_4 = 004,
	Float32   = 011,
	Float32_2 = 012,
	Float32_3 = 013,
	Float32_4 = 014,
	Int8      = 021,
	Int8_2    = 022,
	Int8_3    = 023,
	Int8_4    = 024,
	Uint8     = 031,
	Uint8_2   = 032,
	Uint8_3   = 033,
	Uint8_4   = 034,
	Int16     = 041,
	Int16_2   = 042,
	Int16_3   = 043,
	Int16_4   = 044,
	Uint16    = 051,
	Uint16_2  = 052,
	Uint16_3  = 053,
	Uint16_4  = 054,
	Int32     = 061,
	Int32_2   = 062,
	Int32_3   = 063,
	Int32_4   = 064,
	Uint32    = 071,
	Uint32_2  = 072,
	Uint32_3  = 073,
	Uint32_4  = 074,
	Float10_3 = 005,
};

class VertexBuffer
{
public:
	VertexBuffer(std::initializer_list<VertexType> attributes, const char *name = "");
	~VertexBuffer();

	void UpdateVertices(const void *vertices, uint64_t vertexCount);
	void UpdateIndices(const uint32_t *indices, uint64_t indexCount);
	void Render(uint64_t vertexCount = 0);
private:
	std::string name;

	uint32_t vao         = 0; // Vertex array object
	uint32_t vbo         = 0; // Vertex buffer object
	uint64_t vboSize     = 0;
	uint32_t ebo         = 0; // Element buffer object
	uint64_t eboSize     = 0;

	uint64_t vertexCount = 0;
	uint64_t indexCount  = 0;
	uint32_t stride      = 0;
};