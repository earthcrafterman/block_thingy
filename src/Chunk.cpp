#include "Chunk.hpp"

#include <iostream>
#include <algorithm>		// std::fill_n

#include "Block.hpp"
#include "Cube.hpp"
#include "Game.hpp"
#include "Gfx.hpp"

Chunk::Chunk(Position::ChunkInWorld pos)
	:
	Chunk(pos.x, pos.y, pos.z)
{
}

Chunk::Chunk(ChunkInWorld_type chunk_x, ChunkInWorld_type chunk_y, ChunkInWorld_type chunk_z)
	:
	pos(chunk_x, chunk_y, chunk_z)
{
	this->init();
	std::fill_n(this->blok, CHUNK_BLOCK_COUNT, nullptr);
}

// note: the caller must change the position
Chunk::Chunk(const Chunk& chunk)
	:
	pos(chunk.pos)
{
	this->init();
	for(uint_fast32_t i = 0; i < CHUNK_BLOCK_COUNT; ++i)
	{
		this->blok[i] = chunk.blok[i];
	}
}

Chunk::~Chunk()
{
	delete[] this->blok;
	glDeleteBuffers(1, &this->vbo_v);
	//glDeleteBuffers(1, &this->vbo_e);
}

void Chunk::init()
{
	this->blok = new Block*[CHUNK_BLOCK_COUNT];
	this->changed = true;
	this->vertexes_i = 0;

	//glGenBuffers(1, &this->vbo_e);
	glGenBuffers(1, &this->vbo_v);
}

Block* Chunk::get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

Block* Chunk::get_block(Position::BlockInChunk bcp) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * bcp.y + CHUNK_SIZE * bcp.z + bcp.x];
}

Block* Chunk::get2(int_fast16_t x, int_fast16_t y, int_fast16_t z) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		int64_t bx = this->pos.x * CHUNK_SIZE + x;
		int64_t by = this->pos.y * CHUNK_SIZE + y;
		int64_t bz = this->pos.z * CHUNK_SIZE + z;
		return Game::instance->world.get_block(Position::BlockInWorld(bx, by, bz));
	}
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

void Chunk::set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, Block* block, bool delete_old_block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		// TODO: Block to stream output
		std::cout << "WARNING: Chunk" << this->pos << " ignoring set (" << int(x) << "," << int(y) << "," << int(z) << ") = " << block << "\n";
		return;
	}

	unsigned int index = CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;

	if(delete_old_block)
	{
		Block* old_block = this->blok[index];
		if(old_block != nullptr)
		{
			delete old_block;
		}
	}

	this->blok[index] = block;
	this->changed = true;

	// TODO: make a separate function for the following stuff
	// TODO: is it worth it to check if the naybor chunk has a block beside this one? (to avoid updating when the appearance won't change)
	//std::vector<ChunkInWorld> naybors;
	Position::ChunkInWorld** naybors = new Position::ChunkInWorld*[6];
	int i = 0;
	if(x == 0)
	{
		//ChunkInWorld cp(this->pos.x - 1, this->pos.y, this->pos.z);
		//naybors.push_back(cp);
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x - 1, this->pos.y, this->pos.z);
		naybors[i++] = cp;
	}
	else if(x == CHUNK_SIZE - 1)
	{
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x + 1, this->pos.y, this->pos.z);
		naybors[i++] = cp;
	}
	if(y == 0)
	{
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x, this->pos.y - 1, this->pos.z);
		naybors[i++] = cp;
	}
	else if(y == CHUNK_SIZE - 1)
	{
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x, this->pos.y + 1, this->pos.z);
		naybors[i++] = cp;
	}
	if(z == 0)
	{
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x, this->pos.y, this->pos.z - 1);
		naybors[i++] = cp;
	}
	else if(z == CHUNK_SIZE - 1)
	{
		Position::ChunkInWorld* cp = new Position::ChunkInWorld(this->pos.x, this->pos.y, this->pos.z + 1);
		naybors[i++] = cp;
	}

	for(int j = 0; j < i; ++j)
	{
		Position::ChunkInWorld* cp = naybors[j];
		Chunk* chunk = Game::instance->world.get_chunk(*cp);
		delete cp;
		if(chunk != nullptr)
		{
			chunk->changed = true;
		}
	}
	delete[] naybors;
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
				if(this->get_block(x, y, z) == nullptr)
				{
					continue;
				}
				this->draw_cube(x, y, z, vertexes);
			}
		}
	}
	// TODO: check if vertexes.size() is too big
	this->draw_count = this->vertexes.size();

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

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_v);
	glBufferData(GL_ARRAY_BUFFER, this->vertexes.size() * sizeof(GLfloat), this->vertexes.data(), GL_STATIC_DRAW);

	this->changed = false;
}

void Chunk::render()
{
	if(this->changed)
	{
		this->update();
	}

	glUniform3f(Gfx::vs_cube_pos_mod, this->pos.x * CHUNK_SIZE, this->pos.y * CHUNK_SIZE, this->pos.z * CHUNK_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_v);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, this->draw_count);
	glDisableVertexAttribArray(0);

	glUniform3f(Gfx::vs_cube_pos_mod, 0, 0, 0);
}

void Chunk::add_vertexes(int x, int y, int z, int offset, std::vector<GLfloat>& vertexes)
{
	int cube_pos[] = { x, y, z };
	for(uint_fast8_t i = 0; i < 6; ++i)
	{
		GLuint element = Cube::cube_elements[i + offset * 6];
		for(uint_fast8_t j = 0; j < 3; ++j)
		{
			GLfloat v = Cube::cube_vertex[3 * element + j];
			v += cube_pos[j];
			vertexes.push_back(v);
		}
	}
}

void Chunk::draw_cube(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, std::vector<GLfloat>& vertexes)
{
	// front
	if(this->get2(x, y, z - 1) == nullptr)
	{
		add_vertexes(x, y, z, 0, vertexes);
	}
	// back
	if(this->get2(x, y, z + 1) == nullptr)
	{
		add_vertexes(x, y, z, 1, vertexes);
	}
	// top
	if(this->get2(x, y + 1, z) == nullptr)
	{
		add_vertexes(x, y, z, 2, vertexes);
	}
	// bottom
	if(this->get2(x, y - 1, z) == nullptr)
	{
		add_vertexes(x, y, z, 3, vertexes);
	}
	// right (?)
	if(this->get2(x - 1, y, z) == nullptr)
	{
		add_vertexes(x, y, z, 4, vertexes);
	}
	// left (?)
	if(this->get2(x + 1, y, z) == nullptr)
	{
		add_vertexes(x, y, z, 5, vertexes);
	}
}