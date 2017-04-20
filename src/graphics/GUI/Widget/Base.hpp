#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <json.hpp>
#include <rhea/simplex_solver.hpp>
#include <rhea/variable.hpp>
#include <strict_variant/variant.hpp>

#include "fwd/graphics/GUI/Widget/Component/Base.hpp"
#include "fwd/util/key_mods.hpp"

namespace Graphics::GUI::Widget {

class Base
{
	public:
		Base();
		virtual ~Base();

		virtual std::string type() const = 0;

		virtual void draw();

		virtual void keypress(int key, int scancode, int action, Util::key_mods);
		virtual void charpress(char32_t, Util::key_mods);
		virtual void mousepress(int button, int action, Util::key_mods);
		virtual void mousemove(double x, double y);

		virtual glm::dvec2 get_size() const;
		glm::dvec2 get_position() const;

		void add_modifier(std::shared_ptr<Component::Base>);

		using style_t = std::map<std::string, strict_variant::variant<std::string, double>>;
		using style_vars_t = std::map<std::string, rhea::variable>;

		virtual void read_layout(const json&);
		virtual void apply_layout(rhea::simplex_solver&, style_vars_t& parent_vars);
		virtual void use_layout();

		style_t style;
		style_vars_t style_vars;
		std::string id;

	protected:
		glm::dvec2 size;
		glm::dvec2 position;

		template<typename T>
		T get_layout_var
		(
			const json& layout,
			const std::string& key,
			const T* default_ = nullptr
		);
		template<typename T>
		T get_layout_var
		(
			const json& layout,
			const std::string& key,
			const T& default_
		);

	private:
		std::vector<std::shared_ptr<Component::Base>> modifiers;
};

template<> char Base::get_layout_var(const json&, const std::string&, const char*) = delete;
template<> char Base::get_layout_var(const json&, const std::string&, const char&) = delete;

}
