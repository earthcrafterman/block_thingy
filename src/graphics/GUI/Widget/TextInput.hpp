#pragma once
#include "Base.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "graphics/GUI/Widget/Component/Text.hpp"

namespace block_thingy::graphics::gui::widget {

class TextInput : public Base
{
public:
	TextInput
	(
		const std::string& content = "",
		const std::string& placeholder = ""
	);

	std::string type() const override;

	void draw() override;

	void keypress(const util::key_press&) override;
	void charpress(const util::char_press&) override;
	void mousepress(const util::mouse_press&) override;

	void read_layout(const json&) override;

	std::string get_text() const;
	void set_text(const std::string&);
	void clear();

	void set_focus(bool);

	using on_change_callback_t = std::function<void(TextInput&, const std::string& old_value, const std::string& new_value)>;
	void on_change(on_change_callback_t);
	void trigger_on_change(const std::string& old_value);

	using on_keypress_callback_t = std::function<void(TextInput&, const util::key_press&)>;
	void on_keypress(on_keypress_callback_t);
	void trigger_on_keypress(const util::key_press&);

	bool invalid;

private:
	component::Text content;
	component::Text placeholder;
	bool focus;
	double blink_start_time;
	std::size_t cursor_pos;
	std::size_t selection_start;
	std::vector<on_change_callback_t> on_change_callbacks;
	std::vector<on_keypress_callback_t> on_keypress_callbacks;

	void delete_selection();
};

}
