#pragma once
#include "Engine/Assets/cHandle.h"
#include "cConstantBuffer.h"
#include <iostream>
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Results/Results.h>
#include <Engine/Asserts/Asserts.h>
#include "cShader.h"
#include <string>
#include "cEffect.h"
#include "cTexture.h"

namespace eae6320
{
	namespace Graphics
	{

		class cMaterial
		{
		public:
			cMaterial();
			~cMaterial();


			eae6320::cResult CleanUp();
			eae6320::cResult InitialData(const char filePath[], const char textureFilePath[],Color t_color);


			// Assets
			//-------

			using Handle = Assets::cHandle<cMaterial>;
			static Assets::cManager<cMaterial> s_manager;

			eae6320::Graphics::cEffect::Handle m_effect_handle;
			eae6320::Graphics::cTexture::Handle m_texture_handle;


			Color m_color;
			static cResult Load(const std::string&  i_path, cMaterial*& o_shader, const ShaderTypes::eType i_type);

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cMaterial);
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static eae6320::cResult MaterialFactory(const char* const i_path, Handle &material_handle)
			{
				auto result = Results::Success;

				result = s_manager.Load(i_path,
					material_handle, eae6320::Graphics::ShaderTypes::Vertex);

				return  result;

			}

			uint_fast32_t GetEffectIdx();

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

		};

	}
}


