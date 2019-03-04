#pragma once
#if defined( EAE6320_PLATFORM_WINDOWS )
#include "Direct3D//Includes.h"
#elif
#include "OpenGL/Includes.h"
#endif
#include "cConstantBuffer.h"
#include "ConstantBufferFormats.h"
#include "cRenderState.h"
#include "cShader.h"
#include "sContext.h"
#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>
#include <iostream>
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Results/Results.h>


namespace eae6320
{
	namespace Graphics
	{
		class cEffect
		{
		public:
			cEffect();
			~cEffect();

			eae6320::cResult InitializeShadingData(const char filePath[], const char filePath2[], uint8_t renderStates);
			eae6320::cResult CleanUp();
			void BindShadingData();


			// Assets
			//-------

			using Handle = Assets::cHandle<cEffect>;
			static Assets::cManager<cEffect> s_manager;

			static cResult Load(const std::string& i_path, cEffect*& o_effect_data, const ShaderTypes::eType i_type);


			static eae6320::cResult EffectFactory(const char* const i_path, Handle &effect_handle)
			{
				auto result = Results::Success;

				result = s_manager.Load(i_path,
					effect_handle, eae6320::Graphics::ShaderTypes::Vertex);

				return  result;

			}
		
			




			eae6320::Graphics::cShader::Handle s_vertexShader;
			eae6320::Graphics::cShader::Handle s_fragmentShader;
			eae6320::Graphics::cRenderState s_renderState;
			eae6320::cResult LoadShadingData(const char filePath[], const char filePath2[],uint8_t render_states );
			eae6320::cResult CleanShaderBase();
			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
#ifdef EAE6320_PLATFORM_GL
			GLuint s_programId = 0;
#endif
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect);
		};
	}
}
