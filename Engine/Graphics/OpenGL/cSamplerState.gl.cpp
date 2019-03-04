// Includes
//=========

#include "../cSamplerState.h"

#include "Includes.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cSamplerState::Bind( const unsigned int i_id ) const
{
	// The ID must match the texture layout ID
	EAE6320_ASSERT( m_samplerStateId != 0 );
	glBindSampler( i_id, m_samplerStateId );
	EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSamplerState::Initialize()
{
	auto result = Results::Success;

	const auto samplerStateBits = m_bits;

	constexpr GLsizei samplerStateCount = 1;
	glGenSamplers( samplerStateCount, &m_samplerStateId );
	const auto errorCode = glGetError();
	if ( errorCode == GL_NO_ERROR )
	{
		if ( m_samplerStateId != 0 )
		{
			// Filtering
			{
				GLint parameter_minify, parameter_magnify;
				if ( SamplerStates::IsFilteringEnabled( samplerStateBits ) )
				{
					// Linear filtering for minification, magnification, and between MIP maps ("trilinear filtering")
					parameter_minify = GL_LINEAR_MIPMAP_LINEAR;
					parameter_magnify = GL_LINEAR;
				}
				else
				{
					// Point sampling (the nearest texel is sampled with no blending)
					parameter_minify = GL_NEAREST_MIPMAP_NEAREST;
					parameter_magnify = GL_NEAREST;
				}
				glSamplerParameteri( m_samplerStateId, GL_TEXTURE_MIN_FILTER, parameter_minify );
				{
					const auto errorCode = glGetError();
					if ( errorCode != GL_NO_ERROR )
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to set the sampler state's minifying filter: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						goto OnExit;
					}
				}
				glSamplerParameteri( m_samplerStateId, GL_TEXTURE_MAG_FILTER, parameter_magnify );
				{
					const auto errorCode = glGetError();
					if ( errorCode != GL_NO_ERROR )
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to set the sampler state's magnifying filter: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						goto OnExit;
					}
				}
			}
			// Edge Behavior
			{
				{
					GLint parameter_u;
					{
						const auto edgeBehaviorU = SamplerStates::GetEdgeBehaviorU( samplerStateBits );
						switch ( edgeBehaviorU )
						{
							case SamplerStates::Tile: parameter_u = GL_REPEAT; break;
							case SamplerStates::Clamp: parameter_u= GL_CLAMP; break;

						default:
							EAE6320_ASSERTF( false, "Unrecognized edge behavior %u", edgeBehaviorU );
							parameter_u = GL_CLAMP;
						}
					}
					glSamplerParameteri( m_samplerStateId, GL_TEXTURE_WRAP_S, parameter_u );
					{
						const auto errorCode = glGetError();
						if ( errorCode != GL_NO_ERROR )
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							eae6320::Logging::OutputError( "OpenGL failed to set the sampler state's horizontal wrapping behavior: %s",
								reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							goto OnExit;
						}
					}
				}
				{
					GLint parameter_v;
					{
						const auto edgeBehaviorU = SamplerStates::GetEdgeBehaviorV( samplerStateBits );
						switch ( edgeBehaviorU )
						{
							case SamplerStates::Tile: parameter_v = GL_REPEAT; break;
							case SamplerStates::Clamp: parameter_v= GL_CLAMP; break;

						default:
							EAE6320_ASSERTF( false, "Unrecognized edge behavior %u", edgeBehaviorU );
							parameter_v = GL_CLAMP;
						}
					}
					glSamplerParameteri( m_samplerStateId, GL_TEXTURE_WRAP_T, parameter_v );
					{
						const auto errorCode = glGetError();
						if ( errorCode != GL_NO_ERROR )
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							eae6320::Logging::OutputError( "OpenGL failed to set the sampler state's vertical wrapping behavior: %s",
								reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							goto OnExit;
						}
					}
				}
			}
		}
		else
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERT( false );
			eae6320::Logging::OutputError( "OpenGL failed to create a sampler state" );
			goto OnExit;
		}
	}
	else
	{
		result = eae6320::Results::Failure;
		EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		eae6320::Logging::OutputError( "OpenGL failed to create a sampler state: %s",
			reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		goto OnExit;
	}

OnExit:

	return result;
}

eae6320::cResult eae6320::Graphics::cSamplerState::CleanUp()
{
	auto result = Results::Success;

	if ( m_samplerStateId != 0 )
	{
		constexpr GLsizei samplerStateCount = 1;
		glDeleteSamplers( samplerStateCount, &m_samplerStateId );
		m_samplerStateId = 0;
		const auto errorCode = glGetError();
		if ( errorCode != GL_NO_ERROR )
		{
			if ( result )
			{
				result = Results::Failure;
			}
			EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			eae6320::Logging::OutputError( "OpenGL failed to delete the sampler state: %s",
				reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		}
	}

	return result;
}
