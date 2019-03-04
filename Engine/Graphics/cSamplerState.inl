#ifndef EAE6320_GRAPHICS_CSAMPLERSTATE_INL
#define EAE6320_GRAPHICS_CSAMPLERSTATE_INL

// Includes
//=========

#include "cSamplerState.h"

// Sampler State Bits
//===================

// Filtering
constexpr bool eae6320::Graphics::SamplerStates::IsFilteringEnabled( const uint8_t i_samplerStateBits )
{
	return ( i_samplerStateBits & Filtering ) != 0;
}
constexpr void eae6320::Graphics::SamplerStates::EnableFiltering( uint8_t& io_samplerStateBits )
{
	io_samplerStateBits |= Filtering;
}
constexpr void eae6320::Graphics::SamplerStates::DisableFiltering( uint8_t& io_samplerStateBits )
{
	io_samplerStateBits &= ~Filtering;
}

// Edge  Behavior
constexpr eae6320::Graphics::SamplerStates::eSamplerState eae6320::Graphics::SamplerStates::GetEdgeBehaviorU( const uint8_t i_samplerStateBits )
{
	return ( ( i_samplerStateBits & EdgeBehaviorU ) != 0 ) ? Tile : Clamp;
}
constexpr void eae6320::Graphics::SamplerStates::SetEdgeBehaviorU( const eSamplerState i_edgeBehavior, uint8_t& io_samplerStateBits )
{
	switch ( i_edgeBehavior )
	{
	case Tile:
		{
			io_samplerStateBits |= EdgeBehaviorU;
		}
		break;
	case Clamp:
		{
			io_samplerStateBits &= ~EdgeBehaviorU;
		}
		break;
	}
}
constexpr eae6320::Graphics::SamplerStates::eSamplerState eae6320::Graphics::SamplerStates::GetEdgeBehaviorV( const uint8_t i_samplerStateBits )
{
	return ( ( i_samplerStateBits & EdgeBehaviorV ) != 0 ) ? Tile : Clamp;
}
constexpr void eae6320::Graphics::SamplerStates::SetEdgeBehaviorV( const eSamplerState i_edgeBehavior, uint8_t& io_samplerStateBits )
{
	switch ( i_edgeBehavior )
	{
	case Tile:
		{
			io_samplerStateBits |= EdgeBehaviorV;
		}
		break;
	case Clamp:
		{
			io_samplerStateBits &= ~EdgeBehaviorV;
		}
		break;
	}
}

#endif	// EAE6320_GRAPHICS_CSAMPLERSTATE_INL
