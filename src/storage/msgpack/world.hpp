#pragma once

#include <vector>

#include "game.hpp"
#include "block/BlockRegistry.hpp"
#include "world/world.hpp"

namespace block_thingy::world {

//template<typename Stream>
//void world::save(msgpack::packer<Stream>& o) const
// that does not work :[
template<>
void world::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_array(2);
	o.pack(get_ticks());

	o.pack(block_registry.get_extid_map());
}

template<>
void world::load(const msgpack::object& o)
{
	const auto v = o.as<std::vector<msgpack::object>>();
	ticks = v.at(0).as<decltype(ticks)>();

	// poor design?
	block_registry.set_extid_map(v.at(1).as<block::BlockRegistry::extid_map_t>());
}

}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::world::world;

template<>
struct pack<world>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const world& world) const
	{
		world.save(o);
		return o;
	}
};

template<>
struct convert<world>
{
	const msgpack::object& operator()(const msgpack::object& o, world& world) const
	{
		world.load(o);
		return o;
	}
};

} // namespace msgpack
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace adaptor
