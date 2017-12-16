#include "Settings.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <stdexcept>
#include <type_traits>

#include "console/Console.hpp"
#include "Game.hpp"
#include "event/type/Event_change_setting.hpp"
#include "util/demangled_name.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::settings {

static std::map<string, value_t> settings;

static string get_type_name(const value_t& s)
{
	switch(s.which())
	{
		case 0: return "bool";
		case 1: return "float";
		case 2: return "int";
		case 3: return "string";
		case 4: return "vec2";
		case 5: return "vec3";
		case 6: return "vec4";
	}
	LOG(BUG) << "setting has an unknown type\n";
	return "ERROR";
}

static string get_type_name(const string& name)
{
	return get_type_name(get(name));
}

template<typename T>
string get_type_name_T()
{
	const string name = util::demangled_name<T>();
	LOG(BUG) << "setting has unexpected type " << name << '\n';
	return name;
}
template<> string get_type_name_T<bool      >() { return "bool"  ; }
template<> string get_type_name_T<double    >() { return "float" ; }
template<> string get_type_name_T<int64_t   >() { return "int"   ; }
template<> string get_type_name_T<string    >() { return "string"; }
template<> string get_type_name_T<glm::dvec2>() { return "vec2"  ; }
template<> string get_type_name_T<glm::dvec3>() { return "vec3"  ; }
template<> string get_type_name_T<glm::dvec4>() { return "vec4"  ; }

template<typename T>
bool has(const string& name)
{
	const auto i = settings.find(name);
	if(i == settings.cend())
	{
		return false;
	}
	return i->second.get<T>() != nullptr;
}

template<typename T>
T get(const string& name)
{
	const auto i = settings.find(name);
	if(i == settings.cend())
	{
		throw std::runtime_error("unknown setting name: " + name);
	}
	const T* s = i->second.get<T>();
	if(s == nullptr)
	{
		throw std::runtime_error("can not get " + get_type_name(i->second) + " setting " + name + " as " + get_type_name_T<T>());
	}
	return *s;
}

template<typename T>
void set(const string& name, T value)
{
	if(has<T>(name))
	{
		if(get<T>(name) == value)
		{
			return;
		}

		const value_t old_value = get(name);
		settings[name] = std::move(value);
		if(Game::instance != nullptr)
		{
			Game::instance->event_manager.do_event(Event_change_setting(name, old_value, settings[name]));
		}
	}
	else if(has(name))
	{
		throw std::runtime_error("can not set " + get_type_name(name) + " setting " + name + " to " + get_type_name_T<T>());
	}

	settings.emplace(name, std::move(value));
	// TODO: event
}

bool has(const string& name)
{
	return settings.find(name) != settings.cend();
}

value_t get(const string& name)
{
	const auto i = settings.find(name);
	if(i == settings.cend())
	{
		throw std::runtime_error("unknown setting name: " + name);
	}
	return i->second;
}

void load()
{
	settings =
	{
		{"crosshair_color"		, glm::dvec4(1.0)},
		{"crosshair_size"		, 32},
		{"crosshair_thickness"	, 2},
		{"cull_face"			, true},
		{"cursor_blink_rate"	, 0.5}, // TODO: ensure > 0
		{"far_plane"			, 1500.0},
		{"font"					, "fonts/Anonymous Pro/Anonymous Pro.ttf"},
		{"font_size"			, 24},
		{"fov"					, 75.0},
		{"frustum_culling"		, true},
		{"fullscreen"			, false},
		{"language"				, "en"},
		{"light_smoothing"		, 2},
		{"mesher"				, "Simple"},
		{"min_light"			, 0.005},
		{"near_plane"			, 0.1},
		{"ortho_size"			, 6.0},
		{"projection_type"		, "default"},
		{"render_distance"		, 1},
		{"screen_shader"		, "default"},
		{"show_chunk_outlines"	, false},
		{"show_container_bounds", false},
		{"show_debug_info"		, false},
		{"show_HUD"				, true},
		{"wireframe"			, false},
	};

	Console::instance->run_line("exec settings");
}

// TODO: use smallest representation by using quoted strings
static string format_string(string s)
{
	if(s.empty())
	{
		return "''";
	}
	if(s[0] == '\'' || s[0] == '"')
	{
		s = '\\' + s;
	}
	string s2;
	for(const char c : s)
	{
		     if(c == ' ' ) s2 += "\\ ";
		else if(c == '\t') s2 += "\\t";
		else if(c == '\r') s2 += "\\r";
		else if(c == '\n') s2 += "\\n";
		else
		{
			s2 += c;
		}
	}
	return s2;
}

void save()
{
	std::ofstream f("scripts/settings");
	f << std::boolalpha;
	f.precision(std::numeric_limits<double>::max_digits10);
	for(const auto& p : settings)
	{
		const string name = format_string(p.first);
		const auto& setting = p.second;
		if(const bool* s = setting.get<bool>())
		{
			f << "set_bool " << name << ' ' << *s << '\n';
		}
		else if(const double* s = setting.get<double>())
		{
			f << "set_float " << name << ' ' << *s << '\n';
		}
		else if(const int64_t* s = setting.get<int64_t>())
		{
			f << "set_int " << name << ' ' << *s << '\n';
		}
		else if(const string* s = setting.get<string>())
		{
			f << "set_string " << name << ' ' << format_string(*s) << '\n';
		}
		else if(const glm::dvec2* v = setting.get<glm::dvec2>())
		{
			f << "set_vec2 " << name << ' ' << v->x << ' ' << v->y << '\n';
		}
		else if(const glm::dvec3* v = setting.get<glm::dvec3>())
		{
			f << "set_vec3 " << name << ' ' << v->x << ' ' << v->y << ' ' << v->z << '\n';
		}
		else if(const glm::dvec4* v = setting.get<glm::dvec4>())
		{
			f << "set_vec4 " << name << ' ' << v->x << ' ' << v->y << ' ' << v->z << ' ' << v->w << '\n';
		}
		else
		{
			LOG(BUG) << "unable to save setting " << name << ": not a valid type\n";
		}
	}
}

void add_command_handlers()
{
	Console::instance->add_command("set_bool", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_bool <string: name> <bool: value>\n";
			return;
		}

		const string name = args[0];
		const string value_str = args[1];
		if(value_str != "true" && value_str != "false")
		{
			LOG(ERROR) << "Invalid bool value for " << name << " (must be \"true\" or \"false\")\n";
			return;
		}
		const bool value = (value_str == "true");
		try
		{
			set(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting bool " << name << " = " << value << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set bool: " << name << " = " << value << '\n';
		}
	}});
	Console::instance->add_command("toggle_bool", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: toggle_bool <string: name>\n";
			return;
		}

		const string name = args[0];
		bool value;
		try
		{
			value = !get<bool>(name);
			set(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error toggling bool " << name << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set bool: " << name << " = " << value << '\n';
		}
	}});
	Console::instance->add_command("set_float", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_float <string: name> <number: value or + difference>\n";
			return;
		}

		const string name = args[0];
		const string svalue = args[1];
		double value;
		if(svalue[0] == '+')
		{
			value = has<double>(name) ? get<double>(name) : 0;
			value += std::stod(svalue.substr(1));
		}
		else
		{
			value = std::stod(svalue);
		}

		// TODO: find a home for this
		if(name == "fov")
		{
			if(value < 0)
			{
				value = std::fmod(value, 360) + 360;
			}
			else
			{
				value = std::fmod(value, 360);
			}
			if(value == 0) // avoid division by zero
			{
				value = 360;
			}
		}

		try
		{
			set<double>(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting float " << name << " = " << value << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set float: " << name << " = " << value << '\n';
		}
	}});
	Console::instance->add_command("set_int", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_int <string: name> <integer: value or + difference>\n";
			return;
		}

		const string name = args[0];
		const string svalue = args[1];
		int64_t value;
		if(svalue[0] == '+')
		{
			value = has<int64_t>(name) ? get<int64_t>(name) : 0;
			value += std::stoll(svalue.substr(1));
		}
		else
		{
			value = std::stoll(svalue);
		}

		try
		{
			set<int64_t>(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting int " << name << " = " << value << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set int: " << name << " = " << value << '\n';
		}
	}});
	Console::instance->add_command("set_string", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_string <string: name> <string: value>\n";
			return;
		}

		const string name = args[0];
		const string value = args[1];
		try
		{
			set(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting string " << name << " = " << value << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set string: " << name << " = " << value << '\n';
		}
	}});
	Console::instance->add_command("set_vec2", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 3)
		{
			LOG(ERROR) << "Usage: set_vec2 <string: name> <float: x> <float: y>\n";
			return;
		}

		const string name = args[0];
		const double x = std::stod(args[1]);
		const double y = std::stod(args[2]);
		const glm::dvec2 value(x, y);

		std::ostringstream ss;
		ss.precision(std::numeric_limits<double>::max_digits10);
		ss << '[' << x << ',' << y << ']';
		const string svalue = ss.str();

		try
		{
			set<glm::dvec2>(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting vec2 " << name << " = " << svalue << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set vec2: " << name << " = " << svalue << '\n';
		}
	}});
	Console::instance->add_command("set_vec3", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 4)
		{
			LOG(ERROR) << "Usage: set_vec3 <string: name> <float: x> <float: y> <float: z>\n";
			return;
		}

		const string name = args[0];
		const double x = std::stod(args[1]);
		const double y = std::stod(args[2]);
		const double z = std::stod(args[3]);
		const glm::dvec3 value(x, y, z);

		std::ostringstream ss;
		ss.precision(std::numeric_limits<double>::max_digits10);
		ss << '[' << x << ',' << y << ',' << z << ']';
		const string svalue = ss.str();

		try
		{
			set<glm::dvec3>(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting vec3 " << name << " = " << svalue << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set vec3: " << name << " = " << svalue << '\n';
		}
	}});
	Console::instance->add_command("set_vec4", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 5)
		{
			LOG(ERROR) << "Usage: set_vec4 <string: name> <float: x> <float: y> <float: z> <float: w>\n";
			return;
		}

		const string name = args[0];
		const double x = std::stod(args[1]);
		const double y = std::stod(args[2]);
		const double z = std::stod(args[3]);
		const double w = std::stod(args[4]);
		const glm::dvec4 value(x, y, z, w);

		std::ostringstream ss;
		ss.precision(std::numeric_limits<double>::max_digits10);
		ss << '[' << x << ',' << y << ',' << z << ',' << w << ']';
		const string svalue = ss.str();

		try
		{
			set<glm::dvec4>(name, value);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error setting vec4 " << name << " = " << svalue << ": " << e.what() << '\n';
			return;
		}
		if(Game::instance != nullptr) // not called from initial settings::load()
		{
			LOG(INFO) << "set vec4: " << name << " = " << svalue << '\n';
		}
	}});
}

}
