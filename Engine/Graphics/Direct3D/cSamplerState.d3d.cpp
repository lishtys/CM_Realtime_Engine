// Includes
//=========

#include "../cSamplerState.h"

#include "Includes.h"
#include "../sContext.h"

#include <cfloat>
#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cSamplerState::Bind( const unsigned int i_id ) const
{
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT( direct3dImmediateContext );

	// It is possible (and more efficient) to bind more than one sampler state at a time,
	// and PSSetSamplers() can accept an array of sampler states.
	// Your material could bind sampler states this way since it knows all of the sampler states that need to be bound,
	// but if you use this member function it only knows about itself.
	constexpr unsigned int samplerStateCount = 1;
	// Note that i_id (the "slot") is the sampler state register that you have defined in your shader
	EAE6320_ASSERT( m_samplerState );
	direct3dImmediateContext->PSSetSamplers( i_id, samplerStateCount, &m_samplerState );
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSamplerState::Initialize()
{
	auto result = Results::Success;

	const auto samplerStateBits = m_bits;
	auto* const direct3dDevice = sContext::g_context.direct3dDevice;

	D3D11_SAMPLER_DESC samplerStateDescription{};
	{
		// Filtering
		{
			if ( SamplerStates::IsFilteringEnabled( samplerStateBits ) )
			{
				// Linear filtering for minification, magnification, and between MIP maps ("trilinear filtering")
				samplerStateDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				// Point sampling (the nearest texel is sampled with no blending)
				samplerStateDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			}
		}
		// Edge Behavior
		{
			{
				const auto edgeBehaviorU = SamplerStates::GetEdgeBehaviorU( samplerStateBits );
				switch ( edgeBehaviorU )
				{
					case SamplerStates::Tile: samplerStateDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; break;
					case SamplerStates::Clamp: samplerStateDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; break;

				default:
					EAE6320_ASSERTF( false, "Unrecognized edge behavior %u", edgeBehaviorU );
					samplerStateDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				}
			}
			{
				const auto edgeBehaviorV = SamplerStates::GetEdgeBehaviorV( samplerStateBits );
				switch ( edgeBehaviorV )
				{
					case SamplerStates::Tile: samplerStateDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; break;
					case SamplerStates::Clamp: samplerStateDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; break;

				default:
					EAE6320_ASSERTF( false, "Unrecognized edge behavior %u", edgeBehaviorV );
					samplerStateDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				}
			}
		}
		// Default values
		samplerStateDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerStateDescription.MipLODBias = 0.0f;
		samplerStateDescription.MaxAnisotropy = 1;
		samplerStateDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerStateDescription.BorderColor[0] = samplerStateDescription.BorderColor[1]
			= samplerStateDescription.BorderColor[2] = samplerStateDescription.BorderColor[3] = 1.0f;
		samplerStateDescription.MinLOD = -FLT_MAX;
		samplerStateDescription.MaxLOD = FLT_MAX;
	}
	const auto d3dResult = direct3dDevice->CreateSamplerState( &samplerStateDescription, &m_samplerState );
	if ( FAILED( d3dResult ) )
	{
		result = Results::Failure;
		EAE6320_ASSERTF( false, "CreateSamplerState() failed (HRESULT %#010x)", d3dResult );
		eae6320::Logging::OutputError( "Direct3D failed to create a sampler state with HRESULT %#010x", d3dResult );
		goto OnExit;
	}

OnExit:

	return result;
}

eae6320::cResult eae6320::Graphics::cSamplerState::CleanUp()
{
	auto result = Results::Success;

	if ( m_samplerState )
	{
		m_samplerState->Release();
		m_samplerState = nullptr;
	}

	return result;
}
