#pragma once

#include <glm/vec3.hpp>

#include "fwd/block/enums/Face.hpp"
#include "position/block_in_world.hpp"

namespace block_thingy::physics {

struct raycast_hit
{
	raycast_hit
	(
		const position::block_in_world& pos,
		const glm::ivec3& face
	);

	position::block_in_world pos;
	glm::ivec3 face_vec;

	position::block_in_world adjacent() const;
	block::enums::Face face() const;
};

}
