#pragma once
#include "VertexBuffer.hpp"
#include <vector>
#include <array>
#include <mutex>

// TODO: Greedy meshing

struct Vertex
{
	uint8_t pX, pY, pZ; // Position
	int8_t  nX, nY, nZ; // Normal
	int8_t  tX, tY, tZ;     // Texture coordinate

	Vertex operator+(const glm::vec3 &pos) const
	{
		return {
			(uint8_t)(this->pX + pos.x),
			(uint8_t)(this->pY + pos.y),
			(uint8_t)(this->pZ + pos.z),
			this->nX, this->nY, this->nZ,
			this->tX, this->tY, this->tZ
		};
	}
};

const std::array<Vertex, 6> backFace = {
	Vertex{1, 1, 0,  0,  0, -1, 16, 0 , 1},
	Vertex{1, 0, 0,  0,  0, -1, 16, 16, 1},
	Vertex{0, 0, 0,  0,  0, -1, 0,  16, 1},
	Vertex{0, 0, 0,  0,  0, -1, 0,  16, 1},
	Vertex{0, 1, 0,  0,  0, -1, 0,  0,  1},
	Vertex{1, 1, 0,  0,  0, -1, 16, 0,  1}
};

const std::array<Vertex, 6> frontFace = {
	Vertex{0, 0, 1,  0,  0,  1, 0,  16, 1},
	Vertex{1, 0, 1,  0,  0,  1, 16, 16, 1},
	Vertex{1, 1, 1,  0,  0,  1, 16, 0,  1},
	Vertex{1, 1, 1,  0,  0,  1, 16, 0,  1},
	Vertex{0, 1, 1,  0,  0,  1, 0,  0,  1},
	Vertex{0, 0, 1,  0,  0,  1, 0,  16, 1}
};

const std::array<Vertex, 6> leftFace = {
	Vertex{0, 0, 0, -1,  0,  0, 0,  16, 1},
	Vertex{0, 0, 1, -1,  0,  0, 16, 16, 1},
	Vertex{0, 1, 1, -1,  0,  0, 16, 0,  1},
	Vertex{0, 1, 1, -1,  0,  0, 16, 0,  1},
	Vertex{0, 1, 0, -1,  0,  0, 0,  0,  1},
	Vertex{0, 0, 0, -1,  0,  0, 0,  16, 1}
};

const std::array<Vertex, 6> rightFace = {
	Vertex{1, 1, 1,  1,  0,  0, 16, 0,  1},
	Vertex{1, 0, 1,  1,  0,  0, 16, 16, 1},
	Vertex{1, 0, 0,  1,  0,  0, 0,  16, 1},
	Vertex{1, 0, 0,  1,  0,  0, 0,  16, 1},
	Vertex{1, 1, 0,  1,  0,  0, 0,  0,  1},
	Vertex{1, 1, 1,  1,  0,  0, 16, 0,  1}
};

const std::array<Vertex, 6> bottomFace = {
	Vertex{1, 0, 1,  0, -1,  0, 16, 0,  2},
	Vertex{0, 0, 1,  0, -1,  0, 0,  0,  2},
	Vertex{0, 0, 0,  0, -1,  0, 0,  16, 2},
	Vertex{0, 0, 0,  0, -1,  0, 0,  16, 2},
	Vertex{1, 0, 0,  0, -1,  0, 16, 16, 2},
	Vertex{1, 0, 1,  0, -1,  0, 16, 0,  2}
};

const std::array<Vertex, 6> topFace = {
	Vertex{0, 1, 0,  0,  1,  0, 0,  0,  0},
	Vertex{0, 1, 1,  0,  1,  0, 0,  16, 0},
	Vertex{1, 1, 1,  0,  1,  0, 16, 16, 0},
	Vertex{1, 1, 1,  0,  1,  0, 16, 16, 0},
	Vertex{1, 1, 0,  0,  1,  0, 16, 0,  0},
	Vertex{0, 1, 0,  0,  1,  0, 0,  0,  0}
};

template<int Width, int Height, int Depth, typename VoxelType, VoxelType NullVoxel>
class Chunk
{
public:
	Chunk()
	{
		vertexBuffer = new VertexBuffer({VertexType::Uint8_3, VertexType::Int8_3, VertexType::Int8_3});
		UpdateVertices();
	}

	~Chunk()
	{
		delete vertexBuffer;
	}

	void UpdateVertices()
	{
		vertices.clear();
		for (int y = 0; y < Height; y++) {
			for (int z = 0; z < Depth; z++) {
				for (int x = 0; x < Width; x++) {
					// Back
					if (TestPos(x, y, z) && !TestPos(x, y, z - 1)) {
						for (const auto &vertex : backFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}

					// Front
					if (TestPos(x, y, z) && !TestPos(x, y, z + 1)) {
						for (const auto &vertex : frontFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}

					// Left
					if (TestPos(x, y, z) && !TestPos(x - 1, y, z)) {
						for (const auto &vertex : leftFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}

					// Right
					if (TestPos(x, y, z) && !TestPos(x + 1, y, z)) {
						for (const auto &vertex : rightFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}

					// Bottom
					if (TestPos(x, y, z) && !TestPos(x, y - 1, z)) {
						for (const auto &vertex : bottomFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}

					// Top
					if (TestPos(x, y, z) && !TestPos(x, y + 1, z)) {
						for (const auto &vertex : topFace) {
							vertices.emplace_back(vertex + glm::vec3(x, y, z));
						}
					}
				}
			}
		}
		updated = false;
	}

	void Render()
	{
		if (!updated) {
			vertexBuffer->UpdateVertices(vertices.data(), vertices.size());
			updated  = true;
		}
		vertexBuffer->Render();
	}

	void SetVoxel(int x, int y, int z, VoxelType voxel)
	{
		if (x < 0 || y < 0 || z < 0 || x >= Width || y >= Height || z >= Depth) return;
		const uint64_t index = (y * Width * Depth) + (z * Width) + x;
		voxels[index] = voxel;
	}

	bool TestPos(int x, int y, int z)
	{
		if (x < 0 || y < 0 || z < 0 || x >= Width || y >= Height || z >= Depth) return false;
		const uint64_t index = (y * Width * Depth) + (z * Width) + x;
		return (voxels[index] != NullVoxel);
	}

	std::mutex lock;
	bool       modified = false; // Set to true to prevent chunks from being unloaded
private:
	static_assert(Width  <= 255, "Width cannot exceed 255");
	static_assert(Height <= 255, "Height cannot exceed 255");
	static_assert(Depth  <= 255, "Depth cannot exceed 255");

	std::vector<Vertex> vertices;

	VertexBuffer *vertexBuffer = nullptr;
	VoxelType voxels[Width * Height * Depth] = {NullVoxel};

	bool updated = false;
};