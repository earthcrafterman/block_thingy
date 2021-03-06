#include "Simple.hpp"

#include "game.hpp"
#include "block/base.hpp"
#include "chunk/Chunk.hpp"
#include "position/block_in_chunk.hpp"

namespace block_thingy::mesher {

using block::enums::Face;
using position::block_in_chunk;

meshmap_t Simple::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;
	for(block_in_chunk::value_type x = 0; x < CHUNK_SIZE; ++x)
	for(block_in_chunk::value_type y = 0; y < CHUNK_SIZE; ++y)
	for(block_in_chunk::value_type z = 0; z < CHUNK_SIZE; ++z)
	{
		const block::base& block = *chunk.get_block({x, y, z});
		if(block.is_invisible())
		{
			continue;
		}

		for(uint8_t face_i = 0; face_i < 6; ++face_i)
		{
			const Face face = static_cast<Face>(face_i);
			const Side side = to_side(face);
			const auto i = get_i(face);
			glm::tvec3<int8_t> pos(x, y, z);
			pos[i.y] += static_cast<int8_t>(side);
			if(block_visible_from(chunk, block, pos.x, pos.y, pos.z))
			{
				const auto tex = game::instance->resource_manager.get_block_texture(block.texture(face));
				const meshmap_key_t key =
				{
					block.shader(face),
					block.is_translucent(),
					tex.unit,
				};
				Base::add_face(meshes[key], {x, y, z}, face, 1, 1, tex.index, block.rotation(face));
			}
		}
	}
	return meshes;
}

}
