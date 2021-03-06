#pragma once

#include "util/key_mods.hpp"

namespace block_thingy::util {

struct char_press
{
	char_press(char32_t codepoint, int mods);
	char_press(char32_t codepoint, key_mods);

	char32_t codepoint;
	key_mods mods;
};

}
