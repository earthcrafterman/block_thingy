#include "compiler_info.hpp"

#ifdef _MSC_VER
#include <iomanip>
#include <sstream>
#endif

using std::string;

namespace Util {

string compiler_info()
{
#if defined(__clang__)
	return "Clang " __clang_version__;
#elif defined(__GNUC__)
	return "GCC " __VERSION__;
#elif defined(_MSC_VER)
	std::stringstream s;
	s << std::setfill('0');
	s << std::setw(2) << _MSC_VER / 100 << '.';
	s << std::setw(2) << _MSC_VER % 100 << '.';
	s << std::setw(5) << _MSC_FULL_VER % 100000 << '.';
	s << std::setw(2) << _MSC_BUILD;
	return "MSVC " + s.str();
#else
	return "";
#endif
}

} // namespace Util