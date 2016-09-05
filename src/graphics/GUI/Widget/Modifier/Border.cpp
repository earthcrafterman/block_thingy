#include "Border.hpp"

#include "Game.hpp"
#include "Gfx.hpp"

namespace Graphics::GUI::Widget::Modifier {

Border::Border
(
	const double size,
	const glm::dvec4& color
)
:
	size(size),
	color(color)
{
}

void Border::draw(const Widget::Base& w)
{
	const glm::dvec2 w_size = w.get_size();
	const glm::dvec2 w_position = w.get_real_position();

	const glm::dvec2 border_size_x(w_size.x + size * 2, size);
	const glm::dvec2 border_size_y(size, w_size.y + size * 2);

	const glm::dvec2 border_pos_topleft(w_position - size);
	const glm::dvec2 border_pos_bottom(w_position.x - size, w_position.y + w_size.y);
	const glm::dvec2 border_pos_right(w_position.x + w_size.x, w_position.y - size);

	w.owner.game.gfx.draw_rectangle(border_pos_topleft, border_size_x, color);
	w.owner.game.gfx.draw_rectangle(border_pos_bottom, border_size_x, color);
	w.owner.game.gfx.draw_rectangle(border_pos_topleft, border_size_y, color);
	w.owner.game.gfx.draw_rectangle(border_pos_right, border_size_y, color);
}

} // namespace Graphics::GUI::Widget::Modifier
