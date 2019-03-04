/*
	This file defines the layout of the constant data
	that the CPU sends to the GPU

	These must exactly match the constant buffer definitions in shader programs.
*/

#ifndef EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H
#define EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H

// Includes
//=========

#include "Configuration.h"

#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/sVector.h>
#include "Color.h"

// Format Definitions
//===================

namespace eae6320
{
	namespace Graphics
	{
		namespace ConstantBufferFormats
		{
			struct sPerFrame
			{
				Math::cMatrix_transformation g_transform_worldToCamera;
				Math::cMatrix_transformation g_transform_cameraToProjected;

				Math::sVector g_cameraPosition_world;


				float g_elapsedSecondCount_systemTime = 0.0f;
				float g_elapsedSecondCount_simulationTime = 0.0f;

				Math::sVector g_light_Rot={0,0,1};
				Color g_lightColor ={1,1,1,1};
				Color g_ambientColor = { 0.2f, 0.2f, 0.2f,1.0f };
			};

			struct sPerDrawCall
			{
				Math::cMatrix_transformation g_transform_localToWorld;
				Math::cMatrix_transformation g_transform_localToProject;
			};
			
			struct sPerMaterial
			{
				Color g_tintColor={1,0,0,1};
			};
		}
	}
}

#endif	// EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H
