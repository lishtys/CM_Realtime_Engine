// Includes
//=========

#include "../cTexture.h"

#include "Includes.h"

#include <algorithm>
#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/ScopeCleanUp/cScopeCleanUp.h>

// Helper Function Declarations
//=============================

namespace
{
	constexpr GLenum GetGlFormat( const eae6320::Graphics::TextureFormats::eType i_format );
}

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cTexture::Bind( const unsigned int i_id ) const
{
	// Make the texture unit active
	{
		glActiveTexture( GL_TEXTURE0 + static_cast<GLint>( i_id ) );
		EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
	}
	// Bind the texture to the texture unit
	{
		EAE6320_ASSERT( m_textureId != 0 );
		glBindTexture( GL_TEXTURE_2D, m_textureId );
		EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
	}
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cTexture::Initialize( const std::string& i_path, const void* const i_textureData, const size_t i_textureDataSize )
{
	auto result = Results::Success;

	const cScopeCleanUp scopeCleanUp( [this, &result]
		{
			if ( !result && ( m_textureId != 0 ) )
			{
				constexpr GLsizei textureCount = 1;
				glDeleteTextures( textureCount, &m_textureId );
				EAE6320_ASSERT( glGetError == GL_NO_ERROR );
				m_textureId = 0;
			}
		} );

	// Create a new texture and make it active
	{
		constexpr GLsizei textureCount = 1;
		glGenTextures( textureCount, &m_textureId );
		const auto errorCode = glGetError();
		if ( errorCode == GL_NO_ERROR )
		{
			if ( m_textureId != 0 )
			{
				glBindTexture( GL_TEXTURE_2D, m_textureId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to bind the new texture %u for %s: %s",
						m_textureId, i_path, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					return result;
				}
			}
			else
			{
				result = Results::Failure;
				EAE6320_ASSERT( false );
				eae6320::Logging::OutputError( "OpenGL failed to create a texture for %s", i_path );
				return result;
			}
		}
		else
		{
			result = Results::Failure;
			EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			eae6320::Logging::OutputError( "OpenGL failed to create a texture for %s: %s",
				i_path, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			return result;
		}
	}
	// Fill in the data for each MIP level
	{
		auto currentWidth = static_cast<GLsizei>( m_info.width );
		auto currentHeight = static_cast<GLsizei>( m_info.height );
		auto currentOffset = reinterpret_cast<uintptr_t>( i_textureData );
		const auto finalOffset = currentOffset + i_textureDataSize;
		const auto format = m_info.format;
		const auto blockSize = TextureFormats::GetSizeOfBlock( format );
		const auto glFormat = GetGlFormat( format );
		EAE6320_ASSERT( glFormat != 0 );
		constexpr GLint borderWidth = 0;
		const auto mipMapCount = static_cast<GLint>( m_info.mipMapCount );
		for ( GLint i = 0; i < mipMapCount; ++i )
		{
			// Calculate how much memory this MIP level uses
			size_t singleRowSize, currentMipLevelSize;
			{
				if ( blockSize > 0 )
				{
					// A non-zero block size means that the texture is using block compression
					const auto blockCount_singleRow = ( currentWidth + 3 ) / 4;
					const auto byteCount_singleRow = blockCount_singleRow * blockSize;
					singleRowSize = byteCount_singleRow;
					const auto rowCount = ( currentHeight + 3 ) / 4;
					const auto byteCount_currentMipLevel = byteCount_singleRow * rowCount;
					currentMipLevelSize = byteCount_currentMipLevel;
				}
				else
				{
					// A block of zero size means that the texture is uncompressed
					constexpr auto channelCount = 4;
					constexpr auto byteCount_singleTexel = channelCount * sizeof( uint8_t );
					const auto byteCount_singleRow = currentWidth * byteCount_singleTexel;
					singleRowSize = byteCount_singleRow;
					const auto byteCount_currentMipLevel = byteCount_singleRow * currentHeight;
					currentMipLevelSize = byteCount_currentMipLevel;
				}
			}
			// Set the data into the texture
			if ( TextureFormats::GetCompressionType( format ) != TextureFormats::Compression::None )
			{
				glCompressedTexImage2D( GL_TEXTURE_2D, i, glFormat, currentWidth, currentHeight,
					borderWidth, currentMipLevelSize, reinterpret_cast<void*>( currentOffset ) );
			}
			else
			{
				glTexImage2D( GL_TEXTURE_2D, i, format, currentWidth, currentHeight,
					borderWidth, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<void*>( currentOffset ) );
			}
			const auto errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Update current data for next iteration
				{
					currentOffset += currentMipLevelSize;
					if ( currentOffset <= finalOffset )
					{
						currentWidth = std::max( currentWidth / 2, 1 );
						currentHeight = std::max( currentHeight / 2, 1 );
					}
					else
					{
						result = Results::InvalidFile;
						EAE6320_ASSERTF( false, "Texture file %s is too small to contain MIP map #%i",
							i_path, i );
						Logging::OutputError( "The texture file %s is too small to contain MIP map #%i",
							i_path, i );
						return result;
					}
				}
			}
			else
			{
				result = Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to copy the texture data from MIP map #%i of %s: %s",
					i, i_path, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				return result;
			}
		}
		EAE6320_ASSERTF( currentOffset == finalOffset, "The texture file %s has more texture data (%u) than it should (%u)",
			i_path, finalOffset, currentOffset );
	}
	
	return result;
}

eae6320::cResult eae6320::Graphics::cTexture::CleanUp()
{
	auto result = Results::Success;

	if ( m_textureId != 0 )
	{
		constexpr GLsizei textureCount = 1;
		glDeleteTextures( textureCount, &m_textureId );
		const auto errorCode = glGetError();
		if ( errorCode != GL_NO_ERROR )
		{
			if ( result )
			{
				result = Results::Failure;
			}
			EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			Logging::OutputError( "OpenGL failed to delete the texture %u: %s",
				m_textureId, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		}
		m_textureId = 0;
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	constexpr GLenum GetGlFormat( const eae6320::Graphics::TextureFormats::eType i_format )
	{
		switch ( i_format )
		{
			case eae6320::Graphics::TextureFormats::BC1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case eae6320::Graphics::TextureFormats::BC1_sRGB: return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			case eae6320::Graphics::TextureFormats::BC2: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			case eae6320::Graphics::TextureFormats::BC2_sRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
			case eae6320::Graphics::TextureFormats::BC3: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case eae6320::Graphics::TextureFormats::BC3_sRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			case eae6320::Graphics::TextureFormats::BC4: return GL_COMPRESSED_RED_RGTC1;
			case eae6320::Graphics::TextureFormats::BC5: return GL_COMPRESSED_RG_RGTC2 ;
			case eae6320::Graphics::TextureFormats::BC7: return GL_COMPRESSED_RGBA_BPTC_UNORM;
			case eae6320::Graphics::TextureFormats::BC7_sRGB: return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

			case eae6320::Graphics::TextureFormats::Uncompressed: return GL_RGBA8;
			case eae6320::Graphics::TextureFormats::Uncompressed_sRGB: return GL_SRGB8_ALPHA8;
		}

		// Other formats are possible, but not for our class
		return 0;
	}
}
