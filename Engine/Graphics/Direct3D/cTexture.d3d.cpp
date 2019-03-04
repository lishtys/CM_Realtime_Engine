// Includes
//=========

#include "../cTexture.h"

#include "Includes.h"
#include "../sContext.h"

#include <algorithm>
#include <cstdlib>
#include <dxgiformat.h>
#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/ScopeCleanUp/cScopeCleanUp.h>
#include <new>

// Helper Function Declarations
//=============================

namespace
{
	constexpr DXGI_FORMAT GetDxgiFormat( const eae6320::Graphics::TextureFormats::eType i_format );
}

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cTexture::Bind( const unsigned int i_id ) const
{
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT( direct3dImmediateContext );

	// It is possible (and more efficient) to bind more than one resource at a time,
	// and PSSetShaderResources() can accept an array of shader resource views.
	// Your material could bind textures this way since it knows all of the textures that need to be bound,
	// but if you use this member function it only knows about itself.
	constexpr unsigned int viewCount = 1;
	// Note that i_id (the "slot") is the texture register that you have defined in your shader
	direct3dImmediateContext->PSSetShaderResources( i_id, viewCount, &m_textureView );
}

// Access
//-------

ID3D11ShaderResourceView* eae6320::Graphics::cTexture::GetView() const
{
	return m_textureView;
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cTexture::Initialize( const std::string& i_path, const void* const i_textureData, const size_t i_textureDataSize )
{
	auto result = Results::Success;

	auto* const direct3dDevice = sContext::g_context.direct3dDevice;
	EAE6320_ASSERT( direct3dDevice );

	ID3D11Texture2D* resource = nullptr;
	D3D11_SUBRESOURCE_DATA* subResourceData = nullptr;

	const cScopeCleanUp scopeCleanUp( [&resource, &subResourceData]
		{
			// The texture resource is always released, even on success
			// (the shader resource view holds its own reference to the resource)
			if ( resource )
			{
				resource->Release();
				resource = nullptr;
			}
			if ( subResourceData )
			{
				delete [] subResourceData ;
				subResourceData = nullptr;
			}
		} );

	// Allocate data for a "subresource" for each MIP level
	// (Subresources are the way that Direct3D deals with textures that act like a single resource
	// but that actually have multiple textures associated with that single resource
	// (e.g. MIP maps, volume textures, texture arrays))
	const auto mipMapCount = static_cast<uint_fast8_t>( m_info.mipMapCount );
	{
		subResourceData = new (std::nothrow) D3D11_SUBRESOURCE_DATA[mipMapCount];
		if ( !subResourceData )
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF( false, "Couldn't allocate an array of %u subresource data structs", mipMapCount );
			Logging::OutputError( "Failed to allocate an array of %u subresource data structs for the texture %s",
				mipMapCount, i_path.c_str() );
			return result;
		}
	}
	// Fill in the data for each MIP level
	const auto width = static_cast<uint_fast16_t>( m_info.width );
	const auto height = static_cast<uint_fast16_t>( m_info.height );
	{
		auto currentWidth = width;
		auto currentHeight = height;
		auto currentOffset = reinterpret_cast<uintptr_t>( i_textureData );
		const auto finalOffset = currentOffset + i_textureDataSize;
		const auto blockSize = TextureFormats::GetSizeOfBlock( m_info.format );
		for ( uint_fast8_t i = 0; i < mipMapCount; ++i )
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
			// Set the data into the subresource
			{
				auto& currentSubResourceData = subResourceData[i];
				currentSubResourceData.pSysMem = reinterpret_cast<void*>( currentOffset );
				EAE6320_ASSERT( singleRowSize < std::numeric_limits<decltype(currentSubResourceData.SysMemPitch)>::max() );
				currentSubResourceData.SysMemPitch = static_cast<unsigned int>( singleRowSize );
				EAE6320_ASSERT( currentMipLevelSize < std::numeric_limits<decltype(currentSubResourceData.SysMemSlicePitch)>::max() );
				currentSubResourceData.SysMemSlicePitch = static_cast<unsigned int>( currentMipLevelSize );
			}
			// Update current data for next iteration
			{
				currentOffset += currentMipLevelSize;
				if ( currentOffset <= finalOffset )
				{
					currentWidth = std::max( currentWidth / 2, 1u );
					currentHeight = std::max( currentHeight / 2, 1u );
				}
				else
				{
					result = Results::InvalidFile;
					EAE6320_ASSERTF( false, "Texture file %s is too small to contain MIP map #%u",
						i_path.c_str(), i );
					Logging::OutputError( "The texture file %s is too small to contain MIP map #%u",
						i_path.c_str(), i );
					return result;
				}
			}
		}
		EAE6320_ASSERTF( currentOffset == finalOffset, "The texture file %s has more texture data (%u) than it should (%u)",
			i_path.c_str(), finalOffset, currentOffset );
	}
	// Create the resource
	const auto dxgiFormat = GetDxgiFormat( m_info.format );
	EAE6320_ASSERT( dxgiFormat != DXGI_FORMAT_UNKNOWN );
	{
		D3D11_TEXTURE2D_DESC textureDescription{};
		{
			textureDescription.Width = static_cast<unsigned int>( width );
			textureDescription.Height = static_cast<unsigned int>( height );
			textureDescription.MipLevels = static_cast<unsigned int>( mipMapCount );
			textureDescription.ArraySize = 1;
			textureDescription.Format = dxgiFormat;
			{
				DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;
				sampleDescription.Count = 1;	// No multisampling
				sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
			}
			textureDescription.Usage = D3D11_USAGE_IMMUTABLE;	// The texture will never change once it's been created
			textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// This texture will be used by shaders
			textureDescription.CPUAccessFlags = 0;	// No CPU access is necessary
			textureDescription.MiscFlags = 0;
		}
		const auto d3dResult = direct3dDevice->CreateTexture2D( &textureDescription, subResourceData, &resource );
		if ( FAILED( d3dResult ) )
		{
			result = Results::Failure;
			EAE6320_ASSERTF( false, "CreateTexture2D() failed for %s with HRESULT %#010x", i_path.c_str(), d3dResult );
			eae6320::Logging::OutputError( "Direct3D failed to create a texture from %s with HRESULT %#010x", i_path.c_str(), d3dResult );
			return result;
		}
	}
	// Create the view
	// (a single texture resource could have different "views", meaning that it can be used in different ways
	// as long as it was created with the appropriate BindFlags;
	// this class encapsulates textures that are used by shaders,
	// and so a shader resource view is needed)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription{};
		{
			shaderResourceViewDescription.Format = dxgiFormat;
			shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			{
				D3D11_TEX2D_SRV& shaderResourceView2dDescription = shaderResourceViewDescription.Texture2D;
				shaderResourceView2dDescription.MostDetailedMip = 0;	// Use the highest resolution in the texture resource
				shaderResourceView2dDescription.MipLevels = -1;	// Use all MIP levels
			}
		}
		const auto d3dResult = direct3dDevice->CreateShaderResourceView( resource, &shaderResourceViewDescription, &m_textureView );
		if ( FAILED( d3dResult ) )
		{
			result = Results::Failure;
			EAE6320_ASSERTF( false, "CreateShaderResourceView() failed for %s with HRESULT %#010x", i_path.c_str(), d3dResult );
			eae6320::Logging::OutputError( "Direct3D failed to create a shader resource view for %s with HRESULT %#010x", i_path.c_str(), d3dResult );
			return result;
		}
	}

	return result;
}

eae6320::cResult eae6320::Graphics::cTexture::CleanUp()
{
	auto result = Results::Success;

	if ( m_textureView )
	{
		m_textureView->Release();
		m_textureView = nullptr;
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	constexpr DXGI_FORMAT GetDxgiFormat( const eae6320::Graphics::TextureFormats::eType i_format )
	{
		switch ( i_format )
		{
			case eae6320::Graphics::TextureFormats::BC1: return DXGI_FORMAT_BC1_UNORM;
			case eae6320::Graphics::TextureFormats::BC1_sRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
			case eae6320::Graphics::TextureFormats::BC2: return DXGI_FORMAT_BC2_UNORM;
			case eae6320::Graphics::TextureFormats::BC2_sRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
			case eae6320::Graphics::TextureFormats::BC3: return DXGI_FORMAT_BC3_UNORM;
			case eae6320::Graphics::TextureFormats::BC3_sRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
			case eae6320::Graphics::TextureFormats::BC4: return DXGI_FORMAT_BC4_UNORM;
			case eae6320::Graphics::TextureFormats::BC5: return DXGI_FORMAT_BC5_UNORM;
			case eae6320::Graphics::TextureFormats::BC7: return DXGI_FORMAT_BC7_UNORM;
			case eae6320::Graphics::TextureFormats::BC7_sRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

			case eae6320::Graphics::TextureFormats::Uncompressed: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case eae6320::Graphics::TextureFormats::Uncompressed_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}

		// Other formats are possible, but not for our class
		return DXGI_FORMAT_UNKNOWN;
	}
}
