/*
	This is the standard vertex shader

	A vertex shader is responsible for two things:
		* Telling the GPU where the vertex (one of the three in a triangle) should be drawn on screen in a given window
			* The GPU will use this to decide which fragments (i.e. pixels) need to be shaded for a given triangle
		* Providing any data that a corresponding fragment shader will need
			* This data will be interpolated across the triangle and thus vary for each fragment of a triangle that gets shaded
*/

#include <Shaders/shaders.inc>



#if defined( EAE6320_PLATFORM_D3D )

// Entry Point
//============

void main(

	// Input
	//======

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateInputLayout()

	// These values come from one of the VertexFormats::sMesh that the vertex buffer was filled with in C code
	in const float3 i_vertexPosition_local : POSITION,
	in const float4 i_vertexColor_local : COLOR,
	in const float2 i_uv : TEXCOORD0,
	in const float3 i_normal : NORMAL,
	// Output
	//=======

	// An SV_POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	out float4 o_vertexPosition_projected : SV_POSITION,
	out float4 o_uv_local : COLOR,
	out float4 o_vertexColor_local : COLOR1,
	out float3 o_vertexPosition_local : POSITION,
	out  float3 o_normal : NORMAL

	)


#elif defined( EAE6320_PLATFORM_GL )
// Input
//======
#define mul( i_transform, i_pos ) i_transform*i_pos
#define o_vertexPosition_projected gl_Position
// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// These values come from one of the VertexFormats::sMesh that the vertex buffer was filled with in C code
layout( location = 0 ) in float3 i_vertexPosition_local;
layout(location = 1) in float4 i_vertexColor_local;

// Output
//=======
layout(location = 0) out float4 o_vertexColor_local;

// The vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an automatically-required variable named "gl_Position"

// Entry Point
//============

void main()


#endif


{
	// Transform the local vertex into world space
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is world space.
		float4 vertexPosition_local = float4( i_vertexPosition_local, 1.0 );
		o_vertexPosition_projected = mul(g_transform_localToProject, vertexPosition_local) ;
	}
	// Calculate the position of this vertex projected onto the display
	
	o_uv_local.xy=i_uv.xy;
	o_uv_local.xyzw= o_uv_local.xyxy;
	o_vertexPosition_local=i_vertexPosition_local;
	o_vertexColor_local = i_vertexColor_local;
	const float3x3 rotation_localToWorld = float3x3(
		g_transform_localToWorld[0].xyz,
		g_transform_localToWorld[1].xyz,
		g_transform_localToWorld[2].xyz
		);
	float3 worldNormal= mul(rotation_localToWorld, i_normal);
	o_normal = worldNormal;
}
