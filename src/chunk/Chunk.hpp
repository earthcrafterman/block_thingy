#pragma once

#include <array>
#include <memory>
#include <stdint.h>
#include <vector>

#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "fwd/World.hpp"
#include "block/Base.hpp"
#include "chunk/mesh/Base.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/ChunkInWorld.hpp"

#include "fwd/chunk/Chunk.hpp" // for CHUNK_SIZE and CHUNK_BLOCK_COUNT

using chunk_block_array_t = std::array<std::unique_ptr<Block::Base>, CHUNK_BLOCK_COUNT>;

class Chunk
{
	friend class World;

	public:
		Chunk(const Position::ChunkInWorld&, World& owner);

		Chunk(Chunk&&) = delete;
		Chunk(const Chunk&) = delete;
		void operator=(const Chunk&) = delete;

		World& get_owner() const; // eeh
		Position::ChunkInWorld get_position() const;

		const Block::Base& get_block(Position::BlockInChunk::value_type x, Position::BlockInChunk::value_type y, Position::BlockInChunk::value_type z) const;
		const Block::Base& get_block(const Position::BlockInChunk&) const;
		Block::Base& get_block_m(const Position::BlockInChunk&);

		void set_block(Position::BlockInChunk::value_type x, Position::BlockInChunk::value_type y, Position::BlockInChunk::value_type z, std::unique_ptr<Block::Base>);
		void set_block(const Position::BlockInChunk&, const std::unique_ptr<Block::Base>);

		const Graphics::Color& get_light(const Position::BlockInChunk&) const;
		void set_light(const Position::BlockInChunk&, const Graphics::Color&);

		void update();
		void render(bool transluscent_pass);

		const Mesher::meshmap_t& get_meshes() const;
		void set_meshes(const Mesher::meshmap_t&);

		// for loading
		void set_blocks(std::unique_ptr<chunk_block_array_t>);
		void set_blocks(std::unique_ptr<Block::Base>);

		// for msgpack
		template<typename T> void save(T&) const;
		template<typename T> void load(const T&);

	private:
		World& owner;
		Position::ChunkInWorld position;

		std::unique_ptr<chunk_block_array_t> blocks;
		std::unique_ptr<Block::Base> solid_block;

		std::array<Graphics::Color, CHUNK_BLOCK_COUNT> light;

		bool changed;
		Mesher::meshmap_t meshes;
		std::vector<Graphics::OpenGL::VertexArray> mesh_vaos;
		std::vector<Graphics::OpenGL::VertexBuffer> mesh_vbos;

		void update_vaos();

		void init_block_array();

		void update_neighbors() const;
		void update_neighbors(Position::BlockInChunk::value_type, Position::BlockInChunk::value_type, Position::BlockInChunk::value_type) const;
		void update_neighbor(Position::ChunkInWorld::value_type, Position::ChunkInWorld::value_type, Position::ChunkInWorld::value_type) const;
};
