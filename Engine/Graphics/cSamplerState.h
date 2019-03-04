/*
	Sampler state determines how shaders sample from textures
*/

#ifndef EAE6320_GRAPHICS_CSAMPLERSTATE_H
#define EAE6320_GRAPHICS_CSAMPLERSTATE_H

// Includes
//=========

#include "Configuration.h"

#include <cstdint>
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>

#if defined( EAE6320_PLATFORM_GL )
	#include "OpenGL/Includes.h"
#endif

// Sampler State Bits
//===================

namespace eae6320
{
	namespace Graphics
	{
		namespace SamplerStates
		{
			// The values below are used as bit masks
			enum eSamplerState : uint8_t
			{
				// Set this to enable filtering
				// (when unset point sampling will be used)
				Filtering = 1 << 0,
				// The behavior when sampling outside of [0,1] horizontally
				Clamp = 0,	// The texture will be clamped to the edge value outside of [0,1]
				Tile = 1,	// The texture will repeat outside of [0,1]
				EdgeBehaviorU = 1 << 1,
				EdgeBehaviorV = 1 << 2,
			};

			// Filtering
			constexpr bool IsFilteringEnabled( const uint8_t i_samplerStateBits );
			constexpr void EnableFiltering( uint8_t& io_samplerStateBits );
			constexpr void DisableFiltering( uint8_t& io_samplerStateBits );
			// Edge  Behavior
			constexpr eSamplerState GetEdgeBehaviorU( const uint8_t i_samplerStateBits );
			constexpr void SetEdgeBehaviorU( const eSamplerState i_edgeBehavior, uint8_t& io_samplerStateBits );
			constexpr eSamplerState GetEdgeBehaviorV( const uint8_t i_samplerStateBits );
			constexpr void SetEdgeBehaviorV( const eSamplerState i_edgeBehavior, uint8_t& io_samplerStateBits );
		}
	}
}

// Forward Declarations
//=====================

#if defined( EAE6320_PLATFORM_D3D )
	struct ID3D11SamplerState;
#endif

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cSamplerState
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			// The ID is the HLSL sampler state register
			// or GLSL texture layout ID
			// (note that you decide which sampler state to use with which texture
			// in the shader in Direct3D and in C code in OpenGL).
			// The cSamplerState doesn't know what this is
			// (a single sampler state could be used with many different textures)
			// and so this is the caller's responsibility to pass in.
			void Bind( const unsigned int i_id ) const;

			// Access
			//-------

			using Handle = Assets::cHandle<cSamplerState>;
			static Assets::cManager<cSamplerState, uint8_t> s_manager;

			static constexpr uint8_t InvalidSamplerStateBits = 0xff;

			// Initialization / Clean Up
			//--------------------------

			// The input parameter is a concatenation of SamplerStates::eSamplerState bits
			// which define which sampler states should be enabled
			static cResult Load( const uint8_t i_samplerStateBits, cSamplerState*& o_samplerState );

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS( cSamplerState );

			// Reference Counting
			//-------------------

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			// Data
			//=====

		private:

#if defined( EAE6320_PLATFORM_D3D )
			ID3D11SamplerState* m_samplerState = nullptr;
#else
			GLuint m_samplerStateId = 0;
#endif

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			const uint8_t m_bits = InvalidSamplerStateBits;

			// Implementation
			//===============

			// Initialization / Clean Up
			//--------------------------

			cSamplerState( const uint8_t i_samplerStateBits );
			cResult Initialize();

			cResult CleanUp();
			~cSamplerState();
		};
	}
}

#include "cSamplerState.inl"

#endif	// EAE6320_GRAPHICS_CSAMPLERSTATE_H
