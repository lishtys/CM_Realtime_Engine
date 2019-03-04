// Includes
//=========

#include "cSamplerState.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeCleanUp/cScopeCleanUp.h>
#include <new>

// Static Data Initialization
//===========================

eae6320::Assets::cManager<eae6320::Graphics::cSamplerState, uint8_t> eae6320::Graphics::cSamplerState::s_manager;

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSamplerState::Load( const uint8_t i_samplerStateBits, cSamplerState*& o_samplerState )
{
	auto result = Results::Success;

	EAE6320_ASSERT( o_samplerState == nullptr );

	cSamplerState* newSamplerState = nullptr;

	const cScopeCleanUp scopeCleanUp( [&newSamplerState, &o_samplerState, &result]
		{
			if ( result )
			{
				EAE6320_ASSERT( newSamplerState );
				o_samplerState = newSamplerState;
			}
			else
			{
				if ( newSamplerState )
				{
					newSamplerState->DecrementReferenceCount();
					newSamplerState = nullptr;
				}
				o_samplerState = nullptr;
			}
		} );

	// Allocate a new sampler state
	{
		newSamplerState = new (std::nothrow) cSamplerState( i_samplerStateBits );
		if ( !newSamplerState )
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF( false, "Couldn't allocate memory for a sampler state" );
			Logging::OutputError( "Failed to allocate memory for a sampler state" );
			return result;
		}
	}
	// Initialize the platform-specific sampler state
	if ( !( result = newSamplerState->Initialize() ) )
	{
		EAE6320_ASSERTF( false, "Initialization of new sampler state failed" );
		return result;
	}

	return result;
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::Graphics::cSamplerState::cSamplerState( const uint8_t i_samplerStateBits )
	:
	m_bits( i_samplerStateBits )
{
	EAE6320_ASSERT( i_samplerStateBits != InvalidSamplerStateBits );
}

eae6320::Graphics::cSamplerState::~cSamplerState()
{
	EAE6320_ASSERT( m_referenceCount == 0 );
	const auto result = CleanUp();
	EAE6320_ASSERT( result );
}
