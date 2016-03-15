#include "Game.hpp"

#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Poco/BinaryReader.h>
using Poco::BinaryReader;
#include <Poco/BinaryWriter.h>
using Poco::BinaryWriter;
#include <Poco/DeflatingStream.h>

#include "Block.hpp"
#include "BlockType.hpp"
#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/command_test.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/Event.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/RenderWorld.hpp"
#include "gui/GUI.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/RaytraceHit.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

Game* Game::instance = nullptr;

Game::Game(GLFWwindow* window, const int width, const int height, BinaryReader& reader)
	:
	window(window),
	hovered_block(nullptr),
	cam(window, event_manager),
	gfx(window, event_manager),
	world(reader),
	gui(event_manager),
	delta_time(0),
	fps(144),
	render_distance(3),
	wireframe(false, [](bool wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}),
	keybinder(&console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_test_commands(this);
	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(width, height);
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gfx.set_cam_view(cam);

	player.rotation = cam.rotation;
	player.step(delta_time);
	cam.position = player.position;
	cam.position.y += player.eye_height;

	Position::BlockInWorld render_origin(player.position);
	RenderWorld::draw_world(world, gfx.block_shaders, gfx.matriks, render_origin, render_distance);
	find_hovered_block(gfx.projection_matrix, gfx.view_matrix);
	gui.draw(gfx);

	glfwSwapBuffers(window);
	glfwPollEvents();

	delta_time = fps.enforceFPS();

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	ss << " | player.pos(" << glm::to_string(player.position) << ")";
	Position::BlockInWorld player_block_pos(player.position);
	ss << " | block" << player_block_pos;
	ss << " | chunk" << Position::ChunkInWorld(player_block_pos);
	ss << " | chunkblock" << Position::BlockInChunk(player_block_pos);
	glfwSetWindowTitle(window, ss.str().c_str());
}

#ifdef USE_LIBPNG
void Game::screenshot(const std::string& filename)
{
	std::cout << "saving screenshot to " << filename << "\n";
	std::unique_ptr<GLubyte[]> pixels = std::make_unique<GLubyte[]>(3 * gfx.width * gfx.height);
	glReadPixels(0, 0, gfx.width, gfx.height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), gfx.width, gfx.height, true);
}
#endif

void Game::update_framebuffer_size(const int width, const int height)
{
	event_manager.do_event(Event_window_size_change(width, height));
}

void Game::keypress(const int key, const int scancode, const int action, const int mods)
{
	keybinder.keypress(key, action);
}

static BlockType block_type = BlockType::test;
void Game::mousepress(const int button, const int action, const int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(hovered_block != nullptr && world.get_block_const(hovered_block->pos).type() != BlockType::none)
			{
				auto break_pos = hovered_block->pos;
				world.set_block(break_pos, Block(BlockType::air));
				find_hovered_block(gfx.projection_matrix, gfx.view_matrix);
				event_manager.do_event(Event(EventType::break_block));
			}
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(hovered_block != nullptr)
			{
				Position::BlockInWorld pos = hovered_block->adjacent();
				if(player.can_place_block_at(pos))
				{
					world.set_block(pos, Block(block_type));
				}
			}
		}
	}
}

void Game::mousemove(const double x, const double y)
{
	cam.handleMouseMove(x, y);
}

void Game::find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay(
		glm::vec2(gfx.width / 2.0, gfx.height / 2.0),
		glm::uvec2(gfx.width, gfx.height),
		view_matrix,
		projection_matrix,
		out_origin,
		out_direction
	);

	hovered_block = PhysicsUtil::raycast(world, out_origin, out_direction, 8);
	if(hovered_block != nullptr)
	{
		bool block_is_none = world.get_block_const(hovered_block->pos).type() == BlockType::none;
		glm::vec4 color = block_is_none ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 1, 1);
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	Console* console = &this->console;

	commands.emplace_back(console, "quit", [game=this]()
	{
		std::ofstream stdstream("world.gz", std::ios::binary);
		Poco::DeflatingOutputStream stream(stdstream, Poco::DeflatingStreamBuf::STREAM_GZIP);
		BinaryWriter writer(stream);
		game->world.serialize(writer);
		glfwSetWindowShouldClose(game->window, GL_TRUE);
	});

	// TODO: less copy/paste
	commands.emplace_back(console, "+forward", [game=this]()
	{
		game->player.move_forward(true);
	});
	commands.emplace_back(console, "-forward", [game=this]()
	{
		game->player.move_forward(false);
	});
	commands.emplace_back(console, "+backward", [game=this]()
	{
		game->player.move_backward(true);
	});
	commands.emplace_back(console, "-backward", [game=this]()
	{
		game->player.move_backward(false);
	});
	commands.emplace_back(console, "+left", [game=this]()
	{
		game->player.move_left(true);
	});
	commands.emplace_back(console, "-left", [game=this]()
	{
		game->player.move_left(false);
	});
	commands.emplace_back(console, "+right", [game=this]()
	{
		game->player.move_right(true);
	});
	commands.emplace_back(console, "-right", [game=this]()
	{
		game->player.move_right(false);
	});

	commands.emplace_back(console, "jump", [game=this]()
	{
		game->player.jump();
	});
	commands.emplace_back(console, "noclip", [game=this]()
	{
		game->player.toggle_noclip();
	});
	commands.emplace_back(console, "respawn", [game=this]()
	{
		game->player.respawn();
	});

	commands.emplace_back(console, "save_pos", [game=this](const std::vector<std::string>& args)
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ofstream streem(save_name);
		streem << game->player.position.x << " " << game->player.position.y << " " << game->player.position.z << " ";
		streem << game->player.rotation.x << " " << game->player.rotation.y << " " << game->player.rotation.z;
		streem.flush();
	});
	commands.emplace_back(console, "load_pos", [game=this](const std::vector<std::string>& args)
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ifstream streem(save_name);
		streem >> game->player.position.x;
		streem >> game->player.position.y;
		streem >> game->player.position.z;
		streem >> game->cam.rotation.x;
		streem >> game->cam.rotation.y;
		streem >> game->cam.rotation.z;
	});

	commands.emplace_back(console, "exec", [console=console](const std::vector<std::string>& args)
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::ifstream file("scripts/" + args[0]);
		std::string line;
		while(std::getline(file, line))
		{
			console->run_line(line);
		}
	});

	commands.emplace_back(console, "cam.rot", [cam=&cam](const std::vector<std::string>& args)
	{
		if(args.size() != 2)
		{
			// print usage
			return;
		}
		std::string part = args[0];
		double value = std::stod(args[1]);
		if(part == "x")
		{
			cam->rotation.x += value;
		}
		else if(part == "y")
		{
			cam->rotation.y += value;
		}
		else if(part == "z")
		{
			cam->rotation.z += value;
		}
		else
		{
			// error
		}
	});

	#ifdef USE_LIBPNG
	commands.emplace_back(console, "screenshot", [game=this](const std::vector<std::string>& args)
	{
		std::string filename;
		if(args.size() == 0)
		{
			std::time_t time = std::time(nullptr);
			std::tm t = *std::localtime(&time);
			std::stringstream ss;
			ss << t.tm_year + 1900 << "-";
			ss << std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "-";
			ss << std::setfill('0') << std::setw(2) << t.tm_mday << " ";
			ss << std::setfill('0') << std::setw(2) << t.tm_hour << ":";
			ss << std::setfill('0') << std::setw(2) << t.tm_min << ":";
			ss << std::setfill('0') << std::setw(2) << t.tm_sec;
			ss << " (" << time << ").png";
			filename = ss.str();
		}
		else if(args.size() == 1)
		{
			filename = args[0];
		}
		else
		{
			// print usage
			return;
		}
		try
		{
			game->screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			std::cerr << "error saving screenshot: " << e.what() << "\n";
		}
	});
	#endif

	commands.emplace_back(console, "wireframe", [game=this]()
	{
		game->wireframe = !game->wireframe();
	});
	commands.emplace_back(console, "toggle_cull_face", [game=this]()
	{
		game->gfx.toggle_cull_face();
		std::cout << "cull face: " << (game->gfx.cull_face ? "true" : "false") << "\n";
	});

	commands.emplace_back(console, "render_distance++", [game=this]()
	{
		game->render_distance += 1;
	});
	commands.emplace_back(console, "render_distance--", [game=this]()
	{
		game->render_distance -= 1;
	});

	commands.emplace_back(console, "change_block_type", []()
	{
		block_type_id_t i = static_cast<block_type_id_t>(block_type);
		i = (i + 1) % BlockType_COUNT;
		block_type = static_cast<BlockType>(i);
		std::cout << "block type: " << i << "\n";
	});
	console->run_line("bind j change_block_type");
}
