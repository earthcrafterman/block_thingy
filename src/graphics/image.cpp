#include "image.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-macros"
#endif
#define PNG_SKIP_SETJMP_CHECK // for libpng < 1.5
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
#include <png.h>

#include "util/logger.hpp"

using std::string;
using std::unique_ptr;

namespace block_thingy::graphics {

static void read_png
(
	const fs::path&,
	uint32_t& width,
	uint32_t& height,
	std::vector<uint8_t>& data
);
static void write_png
(
	const fs::path&,
	uint32_t width,
	uint32_t height,
	const uint8_t* data
);

static void write_raw
(
	const fs::path&,
	const std::vector<uint8_t>& data
);

struct image::impl
{
	impl()
	:
		width(0),
		height(0),
		has_transparency(false)
	{
	}

	impl(const fs::path& path)
	{
		if(path.extension() == ".png")
		{
			read_png(path, width, height, data);
		}
		else
		{
			throw std::invalid_argument("unknown image format: " + path.extension().u8string());
		}
		assert(data.size() == 4 * width * height);
		check_transparency();
	}

	impl(const uint32_t width, const uint32_t height, std::vector<uint8_t> data)
	:
		data(std::move(data)),
		width(width),
		height(height)
	{
		if(this->data.size() != 4 * width * height)
		{
			throw std::invalid_argument("bad image data size");
		}
		check_transparency();
	}

	void check_transparency()
	{
		assert(data.size() % 4 == 0);
		for(std::size_t i = 3; i < data.size(); i += 4)
		{
			if(data[i] != 255)
			{
				has_transparency = true;
				return;
			}
		}
		has_transparency = false;
	}

	std::vector<uint8_t> data;
	uint32_t width;
	uint32_t height;
	bool has_transparency;
};

image::image()
:
	pImpl(std::make_unique<impl>())
{
}

image::image(const fs::path& path)
:
	pImpl(std::make_unique<impl>(path))
{
}

image::image(const uint32_t width, const uint32_t height, std::vector<uint8_t> data)
:
	pImpl(std::make_unique<impl>(width, height, data))
{
}

image::~image()
{
}

uint32_t image::get_width() const
{
	return pImpl->width;
}

uint32_t image::get_height() const
{
	return pImpl->height;
}

const uint8_t* image::get_data() const
{
	return pImpl->data.data();
}

bool image::has_transparency() const
{
	return pImpl->has_transparency;
}

void image::write(const fs::path& path) const
{
	if(path.extension() == ".png")
	{
		write_png(path, pImpl->width, pImpl->height, get_data());
	}
	else if(path.extension() == ".raw")
	{
		write_raw(path, pImpl->data);
	}
	else
	{
		throw std::invalid_argument("unknown image format: " + path.extension().u8string());
	}
}

static string png_color_type_name(const int color_type)
{
	switch(color_type)
	{
		case PNG_COLOR_TYPE_GRAY      : return "GRAY";
		case PNG_COLOR_TYPE_PALETTE   : return "PALETTE";
		case PNG_COLOR_TYPE_RGB       : return "RGB";
		case PNG_COLOR_TYPE_RGBA      : return "RGBA";
		case PNG_COLOR_TYPE_GRAY_ALPHA: return "GRAY_ALPHA";
	}
	return std::to_string(color_type);
}

void read_png
(
	const fs::path& path,
	uint32_t& width,
	uint32_t& height,
	std::vector<uint8_t>& data
)
{
	auto file = unique_ptr<std::FILE, decltype(&std::fclose)>(std::fopen(path.string().c_str(), "rb"), &std::fclose);
	if(file == nullptr)
	{
		throw std::runtime_error("unable to read " + path.u8string());
	}
	png_byte header[8];
	const std::size_t png_sig_size = std::fread(header, 1, 8, file.get());
	if(png_sig_size != 8 || png_sig_cmp(header, 0, 8))
	{
		throw std::runtime_error("not a PNG file: " + path.u8string());
	}

	png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

	if(png_ptr == nullptr)
	{
		throw std::runtime_error("png_create_read_struct returned null");
	}

	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == nullptr)
	{
		throw std::runtime_error("png_create_info_struct returned null");
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		throw std::runtime_error("error during png_init_io");
	}

	png_init_io(png_ptr, file.get());
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	const int color_type = png_get_color_type(png_ptr, info_ptr);
	const int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if(bit_depth == 16)
	{
	#if PNG_LIBPNG_VER >= 10504
		png_set_scale_16(png_ptr);
	#else
		png_set_strip_16(png_ptr);
	#endif
	}
	if(color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}
	const bool has_tRNS = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS);
	if(has_tRNS)
	{
		png_set_tRNS_to_alpha(png_ptr);
	}
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}
	if(color_type == PNG_COLOR_TYPE_RGB || (color_type == PNG_COLOR_TYPE_PALETTE && !has_tRNS))
	{
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	const int number_of_passes = png_set_interlace_handling(png_ptr); // is this needed for reading?
	png_read_update_info(png_ptr, info_ptr);

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		throw std::runtime_error("error while reading PNG file " + path.u8string());
	}

	std::size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	LOG(DEBUG) << path.u8string() << ": " << width << "×" << height
			   << "; bit depth = " << bit_depth
			   << "; color type = " << png_color_type_name(color_type)
			   << "; pass count = " << number_of_passes
			   << "; rowbytes = " << rowbytes
			   << '\n'
			;

	data.resize(height * rowbytes);
	for(std::size_t y = height; y > 0; --y)
	{
		png_read_row(png_ptr, &data[(y - 1) * rowbytes], nullptr);
	}
}

void write_png
(
	const fs::path& path,
	const uint32_t width,
	const uint32_t height,
	const uint8_t* data
)
{
	// http://www.libpng.org/pub/png/spec/iso/index-object.html#3PNGfourByteUnSignedInteger
	// http://www.libpng.org/pub/png/spec/iso/index-object.html#11IHDR
	if(width == 0 || width > std::numeric_limits<int32_t>::max())
	{
		throw std::invalid_argument("width");
	}
	if(height == 0 || height > std::numeric_limits<int32_t>::max())
	{
		throw std::invalid_argument("height");
	}

	png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(png_ptr == nullptr)
	{
		throw std::runtime_error("png_create_write_struct returned null");
	}
	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == nullptr)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, nullptr);
		throw std::runtime_error("png_create_info_struct returned null");
	}
	FILE* fp = fopen(path.string().c_str(), "wb");
	if(fp == nullptr)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw std::runtime_error(string("error opening png file for writing: ") + std::strerror(errno));
	}
	png_init_io(png_ptr, fp);
	const int bit_depth = 8;
	const uint32_t w = static_cast<uint32_t>(width);
	const uint32_t h = static_cast<uint32_t>(height);
	png_set_IHDR(png_ptr, info_ptr, w, h, bit_depth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	const std::size_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
	for(std::size_t y = height; y > 0; --y)
	{
		png_write_row(png_ptr, const_cast<uint8_t*>(data + (y - 1) * rowsize));
	}
	png_write_end(png_ptr, info_ptr);
	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void write_raw
(
	const fs::path& path,
	const std::vector<uint8_t>& data
)
{
	assert(data.size() <= std::numeric_limits<std::streamsize>::max());
	std::ofstream f(path, std::ofstream::binary);
	f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

}
