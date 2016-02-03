#include "Chunk.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include "Block.hpp"
#include "Coords.hpp"
#include "Cube.hpp"
#include "World.hpp"

Chunk::Chunk(Position::ChunkInWorld pos, World* owner)
	:
	owner(owner),
	pos(pos)
{
	this->init();
}

// note: the caller must change the position
Chunk::Chunk(const Chunk& chunk)
	:
	owner(chunk.owner),
	pos(chunk.pos),
	blok(chunk.blok)
{
	this->init();
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &this->mesh_vbo);
}

void Chunk::init()
{
	this->changed = true;
	this->vertexes_i = 0;

	glGenBuffers(1, &this->mesh_vbo);
}

Block Chunk::get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

Block Chunk::get_block(Position::BlockInChunk bcp) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * bcp.y + CHUNK_SIZE * bcp.z + bcp.x];
}

Block Chunk::get2(int_fast16_t x, int_fast16_t y, int_fast16_t z) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		int64_t bx = this->pos.x * CHUNK_SIZE + x;
		int64_t by = this->pos.y * CHUNK_SIZE + y;
		int64_t bz = this->pos.z * CHUNK_SIZE + z;
		return this->owner->get_block(Position::BlockInWorld(bx, by, bz));
	}
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

void Chunk::set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, Block block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::string set_info = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") = " + std::to_string(block.type());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	uint_fast32_t index = CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;

	this->blok[index] = block;
	this->changed = true;

	this->update_neighbors(x, y, z);
}

/** TODO: figure out what causes the weird chunk rendering error
INFO:
	it happens on both of my laptop computers (Intel graphics cards, GNU/Linux (3.3) and Windows (3.0))
	zel says it does not happen with his AMD card (Windows (up to 4.4))
	zel notes: 'there is a fine difference between fixing a bug and breaking app so the bug does not appear'
*/
void Chunk::update()
{
	#ifdef WEIRD_BUG_FIX
	auto old_size = this->vertexes.size();
	#endif

	this->vertexes.clear();
	for(BlockInChunk_type x = 0; x < CHUNK_SIZE; ++x)
	{
		for(BlockInChunk_type y = 0; y < CHUNK_SIZE; ++y)
		{
			for(BlockInChunk_type z = 0; z < CHUNK_SIZE; ++z)
			{
				if(this->get_block(x, y, z).type() == 0)
				{
					continue;
				}
				this->draw_cube(x, y, z, vertexes);
			}
		}
	}
	this->draw_count = static_cast<GLsizei>(this->vertexes.size());

	#ifdef WEIRD_BUG_FIX
	if(this->vertexes.size() < old_size)
	{
		this->vertexes.reserve(old_size);
		while(this->vertexes.size() < old_size)
		{
			this->vertexes.push_back(0);
		}
	}
	#endif

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertexes.size() * sizeof(GLfloat), this->vertexes.data(), GL_DYNAMIC_DRAW);

	this->changed = false;
}

void Chunk::render()
{
	if(this->changed)
	{
		this->update();
	}

	glUniform3f(this->owner->vs_cube_pos_mod, this->pos.x * CHUNK_SIZE, this->pos.y * CHUNK_SIZE, this->pos.z * CHUNK_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->mesh_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, this->draw_count);
	glDisableVertexAttribArray(0);

	//glUniform3f(this->owner->vs_cube_pos_mod, 0, 0, 0);
}

void Chunk::update_neighbors(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z)
{
	// TODO: check if the neighbor chunk has a block beside this one (to avoid updating when the appearance won't change)
	if(x == 0)
	{
		this->update_neighbor(-1, 0, 0);
	}
	else if(x == CHUNK_SIZE - 1)
	{
		this->update_neighbor(+1, 0, 0);
	}
	if(y == 0)
	{
		this->update_neighbor(0, -1, 0);
	}
	else if(y == CHUNK_SIZE - 1)
	{
		this->update_neighbor(0, +1, 0);
	}
	if(z == 0)
	{
		this->update_neighbor(0, 0, -1);
	}
	else if(z == CHUNK_SIZE - 1)
	{
		this->update_neighbor(0, 0, +1);
	}
}

void Chunk::update_neighbor(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z)
{
	Position::ChunkInWorld cp(x + this->pos.x, y + this->pos.y, z + this->pos.z);
	std::shared_ptr<Chunk> chunk = this->owner->get_chunk(cp);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}

void Chunk::add_vertexes(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t offset, std::vector<GLfloat>& vertexes)
{
	offset *= 6;
	for(uint_fast8_t i = 0; i < 6; ++i)
	{
		uint_fast8_t element = 3 * Cube::cube_elements[offset++];
		vertexes.push_back(Cube::cube_vertex[element++] + x);
		vertexes.push_back(Cube::cube_vertex[element++] + y);
		vertexes.push_back(Cube::cube_vertex[element++] + z);
	}
}

void Chunk::draw_cube(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, std::vector<GLfloat>& vertexes)
{
	// front
	if(this->get2(x, y, z - 1).type() == 0)
	{
		add_vertexes(x, y, z, 0, vertexes);
	}
	// back
	if(this->get2(x, y, z + 1).type() == 0)
	{
		add_vertexes(x, y, z, 1, vertexes);
	}
	// top
	if(this->get2(x, y + 1, z).type() == 0)
	{
		add_vertexes(x, y, z, 2, vertexes);
	}
	// bottom
	if(this->get2(x, y - 1, z).type() == 0)
	{
		add_vertexes(x, y, z, 3, vertexes);
	}
	// right (?)
	if(this->get2(x - 1, y, z).type() == 0)
	{
		add_vertexes(x, y, z, 4, vertexes);
	}
	// left (?)
	if(this->get2(x + 1, y, z).type() == 0)
	{
		add_vertexes(x, y, z, 5, vertexes);
	}
}