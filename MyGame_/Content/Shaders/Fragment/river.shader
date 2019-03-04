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
	// Output as input color
	
			float2 scrolledUV  =  i_uv.xy;
		//	float xScrollValue = 1 * g_elapsedSecondCount_simulationTime;  //4
           // float yScrollValue = 1 * g_elapsedSecondCount_simulationTime;
	//scrolledUV+=float2(xScrollValue,0);
	float4 justColor = SampleTexture2d(g_diffuse_texture, g_diffuse_samplerState, scrolledUV.xy);
//	o_color =i_color*g_tintColor;
	o_color = justColor * g_tintColor*i_color;
}
