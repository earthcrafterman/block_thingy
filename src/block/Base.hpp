#pragma once

#include "block/BlockType.hpp"
#include "fwd/block/BlockVisibilityType.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/storage/Interface.hpp"

namespace Block {

class Base
{
	public:
		Base();
		explicit Base(block_type_id_t);
		explicit Base(BlockType);
		virtual ~Base();

		Base(const Base&);
		virtual void operator=(const Base&);

		block_type_id_t type_id() const;
		BlockType type() const;

		/**
		 * If non-zero, this block emits light of this color
		 */
		virtual Graphics::Color color() const;

		virtual double bounciness() const;

		virtual BlockVisibilityType visibility_type() const;
		bool is_opaque() const;
		bool is_translucent() const;
		bool is_invisible() const;

		/**
		 * Can entities collide with this block?
		 */
		virtual bool is_solid() const;

		/**
		 * Can players interact with this block?
		 */
		virtual bool is_selectable() const;

		virtual void save(Storage::OutputInterface&) const;
		virtual void load(Storage::InputInterface&);

	private:
		BlockType type_;
};

} // namespace Block
