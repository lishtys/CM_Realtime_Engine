/*
	This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

	// GLSL shaders require the version to be #defined before anything else in the shader
	#version 420

#endif

// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )
    
	#define DeclareConstantBuffer( i_name, i_id ) cbuffer i_name : register( b##i_id )
	#define DeclareTexture2d( i_name, i_id ) Texture2D i_name : register( t##i_id )
	#define DeclareSamplerState( i_name, i_id ) SamplerState i_name : register( s##i_id )

	#define SampleTexture2d( i_texture, i_samplerState, i_uv ) i_texture.Sample( i_samplerState, i_uv )
	
	#define CalculateLighting_multiplicative( i_normal_world )  g_lightColor*saturate(dot( - normalize(g_light_Rot), normalize(i_normal)))

#elif defined( EAE6320_PLATFORM_GL )
	
	#define float4x4 mat4
	#define float2 vec2 
	#define float3 vec3 
	#define float4 vec4 

	#define DeclareConstantBuffer( i_name, i_id ) layout( std140, binding = i_id ) uniform i_name

	#define DeclareTexture2d( i_name, i_id ) layout( binding = i_id ) uniform sampler2D i_name
	#define DeclareSamplerState( i_name, i_id )	// GLSL requires the sampler state to be set in C code

	#define SampleTexture2d( i_texture, i_samplerState, i_uv ) texture2D( i_texture, i_uv )
#endif

DeclareConstantBuffer( g_constantBuffer_perFrame, 0)
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToProjected;

	float3 g_cameraPosition_world;
	

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;

	float3 g_light_Rot;
	float4 g_lightColor;
	float4 g_ambientColor;
};

DeclareConstantBuffer( g_constantBuffer_perDrawCall, 2)
{
  float4x4 g_transform_localToWorld;
  float4x4 g_transform_localToProject;
};

DeclareConstantBuffer( g_constantBuffer_perMaterial, 1)
{
	float4 g_tintColor;
  
};

DeclareTexture2d(g_diffuse_texture,0);
DeclareSamplerState(g_diffuse_samplerState,0);