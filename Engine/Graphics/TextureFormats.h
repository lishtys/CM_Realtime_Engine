/*
	A texture format determines the layout of the texture data
	that the CPU will send to the GPU
*/

#ifndef EAE6320_GRAPHICS_TEXTUREFORMATS_H
#define EAE6320_GRAPHICS_TEXTUREFORMATS_H

// Includes
//=========

#include "Configuration.h"

#include <cstdint>

// Texture Formats
//================

namespace eae6320
{
	namespace Graphics
	{
		namespace TextureFormats
		{
			// Most graphics hardware natively supports a specific kind of texture compression
			// (formerly known as "DXT" or "S3TC", but now more commonly called "BC" for "block compression").
			// This isn't a particularly efficient compression scheme
			// (compared to the more common image formats you may know),
			// but since it can be decompressed for free by graphics hardware
			// it is a good choice for games.
			// For a good explanation see:
			//	http://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/
			namespace Compression
			{
				// Note that all of the texture formats in our class will give a [0,1] value when sampled
				enum eType : uint8_t
				{
					// RGB, 8 bytes per 4x4 block
					BC1,
					// RGB + limited A, 16 bytes per 4x4 block (BC1 for RGB + 4 uncompressed bits for each alpha pixel; use BC3 or BC7 instead)
					BC2,
					// RGBA, 16 bytes per 4x4 block (BC1 for RGB + BC4 for A)
					BC3,
					// Grayscale, 8 bytes per 4x4 block
					BC4,
					// 2 channel grayscale, 16 bytes per 4x4 block (2 * BC4)
					BC5,
					// RGBA, 16 bytes per 4x4 block (better quality than equally-sized BC3, but slow to encode)
					BC7,

					// In our class this is RGBA, 8 bits per channel (i.e. 32 bits per texel)
					None,

					Unknown
				};
			}

			// Graphics hardware can natively support sRGB encoding and decoding;
			// this sRGB bit can be combined with the compression type to specify a texture format
			constexpr uint8_t sRGB_bit = 0x8;

			enum eType : uint8_t
			{
				Uncompressed = Compression::None,
				Uncompressed_sRGB = Compression::None | sRGB_bit,

				BC1 = Compression::BC1,
				BC1_sRGB = Compression::BC1 | sRGB_bit,
				BC2 = Compression::BC2,
				BC2_sRGB = Compression::BC2 | sRGB_bit,
				BC3 = Compression::BC3,
				BC3_sRGB = Compression::BC3 | sRGB_bit,
				BC4 = Compression::BC4,
				BC5 = Compression::BC5,
				BC7 = Compression::BC7,
				BC7_sRGB = Compression::BC7 | sRGB_bit,

				Unknown = Compression::Unknown
			};

			inline constexpr Compression::eType GetCompressionType( const eType i_format )
			{
				return static_cast<Compression::eType>( i_format & ~sRGB_bit );
			}

			// Returns the size in bytes of a single 4x4 (16 pixel) block
			inline constexpr unsigned int GetSizeOfBlock( const eType i_format )
			{
				// Only the compression matters (sRGB is irrelevant)
				switch ( GetCompressionType( i_format ) )
				{
					// RGB (with an optional binary alpha)
					case Compression::BC1: return 8;
					// RGB + A (BC1 for RGB and 4 bits for each alpha pixel)
					case Compression::BC2: return GetSizeOfBlock( BC1 ) + ( ( 4 * 16 ) / 8 );
					// RGB + A (BC1 for RGB and BC4 for alpha)
					case Compression::BC3: return GetSizeOfBlock( BC1 ) + GetSizeOfBlock( BC4 );
					// Single channel
					case Compression::BC4: return 8;
					// Two channels (2 BC4s)
					case Compression::BC5: return GetSizeOfBlock( BC4 ) + GetSizeOfBlock( BC4 );
					// RGBA
					case Compression::BC7: return 16;

				// The format doesn't use block compression
				default:
					return 0;
				}
			}

			// This struct is a binary description of the texture that is stored in a texture file
			// and loaded and used at run-time
			struct sTextureInfo
			{
				uint16_t width, height;
				uint8_t mipMapCount;
				eType format;
			};
		}
	}
}

#endif	//EAE6320_GRAPHICS_TEXTUREFORMATS_H
