#include "RaytraceHit.hpp"

#include "../position/BlockInWorld.hpp"

RaytraceHit::RaytraceHit(const Position::BlockInWorld& pos, const glm::ivec3& face)
	:
	pos(pos),
	face(face)
{
}

Position::BlockInWorld RaytraceHit::adjacent()
{
	return pos + face;
}
