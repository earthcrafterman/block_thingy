#pragma once
#include "frustum.hpp"

namespace block_thingy::graphics {

template<bool B>
class null_frustum : public frustum
{
public:
	bool inside(const glm::dvec3&) const override
	{
		return B;
	}

	bool inside(const physics::AABB&) const override
	{
		return B;
	}
};

}
