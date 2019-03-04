/*
	This is the standard fragment shader

	A fragment shader is responsible for telling the GPU what color a specific fragment should be
*/

#include <Shaders/shaders.inc>

//DeclareConstantBuffer_perFrame(g_constantBuff_perFrame, 0);
//DeclareConstantBuffer_perDrawCall(g_constantBuff_perDrawCall, 2);

#if defined( EAE6320_PLATFORM_D3D )

// Entry Point
//============

void main(

	// Input
	in const float4 i_position : SV_POSITION,
	in const float4 i_uv : COLOR,
	in const float4 i_color : COLOR1,
	in const float3 i_normal : NORMAL,
	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : SV_TARGET

	)

#elif defined( EAE6320_PLATFORM_GL )

//Input
layout(location = 0) in float4 i_color;


// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out float4 o_color;

// Entry Point
//============

void main()

#endif
{

	float4 color_directional = CalculateLighting_multiplicative(i_normal);
	// Output as input color
	float4 textureColor = SampleTexture2d(g_diffuse_texture, g_diffuse_samplerState, i_uv.xy);

	float4 color_multiplicativeLighting = saturate((color_directional + g_ambientColor).rgba);

	o_color = textureColor * g_tintColor*i_color*color_multiplicativeLighting;
}
