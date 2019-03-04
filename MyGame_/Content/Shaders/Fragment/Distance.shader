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
	in const float4 i_vertPosition : POSITION,	
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
	float offset = g_cameraPosition_world.z-i_position.z;

	float s = offset/ 10;

	float4 closeColor = float4(
		// RGB (color)
		1.0, 0.0, 1.0,
		// Alpha (transparency)
		1.0);
	
	float4 calculateColor = float4(
		// RGB (color)
		1.0, 1.0, 1.0,
		// Alpha (transparency)
		1.0);
	calculateColor.rgb = lerp(closeColor.rgb, calculateColor.rgb, s);

	float4 combinedColor=	calculateColor*i_color;
	o_color = combinedColor;

}
