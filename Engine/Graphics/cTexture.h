/*
	A texture is an n-dimensional array of data
	(usually a 2-dimensional array of colors)
	that can be sampled in a shader
*/

#ifndef EAE6320_GRAPHICS_CTEXTURE_H
#define EAE6320_GRAPHICS_CTEXTURE_H

// Includes
//=========

#include "Configuration.h"

#include "TextureFormats.h"

#include <cstdint>
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>
#include <string>

#ifdef EAE6320_PLATFORM_GL
	#include "OpenGL/Includes.h"
#endif

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
	struct ID3D11ShaderResourceView;
#endif

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cTexture
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			// The ID is the HLSL register or GLSL layout ID
			// defined in the shader that uses this texture.
			// The cTexture doesn't know what this is
			// (a single texture could be used by many different effects)
			// and so this is the caller's responsibility to pass in.
			void Bind( const unsigned int i_id ) const;

			// Access
			//-------

			using Handle = Assets::cHandle<cTexture>;
			static Assets::cManager<cTexture> s_manager;

			uint16_t GetWidth() const;
			uint16_t GetHeight() const;

#if defined( EAE6320_PLATFORM_D3D )
			ID3D11ShaderResourceView* GetView() const;
#endif

			// Initialization / Clean Up
			//--------------------------

			static cResult Load( const std::string& i_path, cTexture*& o_texture );

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS( cTexture );

			// Reference Counting
			//-------------------

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			// Data
			//=====

		private:

#if defined( EAE6320_PLATFORM_D3D )
			ID3D11ShaderResourceView* m_textureView = nullptr;
#elif defined( EAE6320_PLATFORM_GL )
			GLuint m_textureId = 0;
#endif

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			TextureFormats::sTextureInfo m_info;

			// Implementation
			//===============

		private:

			// Initialization / Clean Up
			//--------------------------

			cTexture( const TextureFormats::sTextureInfo i_info );
			cResult Initialize( const std::string& i_path_effect, const void* const i_textureData, const size_t i_textureDataSize );

			~cTexture();
			cResult CleanUp();
		};
	}
}

#endif	// EAE6320_GRAPHICS_CTEXTURE_H
