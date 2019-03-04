/*
	This is the standard fragment shader

	A fragment shader is responsible for telling the GPU what color a specific fragment should be
*/

#include <Shaders/shaders.inc>

#if defined( EAE6320_PLATFORM_D3D )

// Constant Buffers
//=================



// Entry Point
//============

void main(

	in const float4 i_position : SV_POSITION,
	in const float4 i_color : COLOR,	
	in const float3 i_vertPosition : POSITION,	
	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : SV_TARGET
	)

#elif defined( EAE6320_PLATFORM_GL )

// Constant Buffers
//=================

layout( location = 0 ) in float4 i_color;
// Output
//=======
// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out float4 o_color;
in float depthZ;
// Entry Point
//============

void main()


#endif
{
	// Output solid white

	float bVal = 1.0;
	

	float rVal = floor(frac(i_position.x) + 0.5);
	float gVal= floor(frac(i_position.y) + 0.5);
//	rVal = sin(i_position.x);
	gVal = sin(i_position.y);
	bVal = sin(i_position.z);
	float4 calculateColor = float4(
		// RGB (color)
		rVal, gVal, bVal,
		// Alpha (transparency)
		1.0);
	float4 combinedColor=	calculateColor*i_color;
	o_color = combinedColor;

}
