#include "SimpleMesher.hpp"

#include <vector>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../Coords.hpp"
#include "../../Cube.hpp"

SimpleMesher::SimpleMesher(const Chunk& chunk)
	:
	ChunkMesher(chunk)
{
}

meshmap_t SimpleMesher::make_mesh()
{
	meshmap_t meshes;
	for(BlockInChunk_type x = 0; x < CHUNK_SIZE; ++x)
	{
		for(BlockInChunk_type y = 0; y < CHUNK_SIZE; ++y)
		{
			for(BlockInChunk_type z = 0; z < CHUNK_SIZE; ++z)
			{
				Block block = block_at(x, y, z);
				if(block.is_invisible())
				{
					continue;
				}
				BlockType type = block.type();
				mesh_t& mesh = meshes[type];
				draw_cube(mesh, x, y, z);
			}
		}
	}
	return meshes;
}

void SimpleMesher::draw_cube(mesh_t& mesh, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z)
{
	// front
	if(block_is_invisible(x, y, z - 1))
	{
		draw_face(mesh, x, y, z, 0);
	}
	// back
	if(block_is_invisible(x, y, z + 1))
	{
		draw_face(mesh, x, y, z, 1);
	}
	// top
	if(block_is_invisible(x, y + 1, z))
	{
		draw_face(mesh, x, y, z, 2);
	}
	// bottom
	if(block_is_invisible(x, y - 1, z))
	{
		draw_face(mesh, x, y, z, 3);
	}
	// right (?)
	if(block_is_invisible(x - 1, y, z))
	{
		draw_face(mesh, x, y, z, 4);
	}
	// left (?)
	if(block_is_invisible(x + 1, y, z))
	{
		draw_face(mesh, x, y, z, 5);
	}
}

void SimpleMesher::draw_face(mesh_t& mesh, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t face)
{
	auto offset = face * 6;
	for(uint_fast8_t i = 0; i < 2; ++i)
	{
		mesh_triangle_t tri;
		for(uint_fast8_t j = 0; j < 3; ++j)
		{
			uint_fast8_t element = 3 * Cube::cube_elements[offset++];
			tri[j].x = Cube::cube_vertex[element++] + x;
			tri[j].y = Cube::cube_vertex[element++] + y;
			tri[j].z = Cube::cube_vertex[element++] + z;
		}
		mesh.push_back(tri);
	}
}
