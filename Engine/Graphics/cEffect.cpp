
#include "Engine/Asserts/Asserts.h"
#include "Tools/ShaderBuilder/cShaderBuilder.h"
#include "cEffect.h"
eae6320::Assets::cManager<eae6320::Graphics::cEffect> eae6320::Graphics::cEffect::s_manager;

namespace eae6320
{
	namespace Graphics
	{
		eae6320::cResult cEffect::LoadShadingData(const char filePath[], const char filePath2[],uint8_t render_state)
		{
			auto result = eae6320::Results::Success;
			if (!(result = eae6320::Graphics::cShader::s_manager.Load(filePath,
				s_vertexShader, eae6320::Graphics::ShaderTypes::Vertex)))
			{
				EAE6320_ASSERT(false);
				return result;
			}
			if (!(result = eae6320::Graphics::cShader::s_manager.Load(filePath2,
				s_fragmentShader, eae6320::Graphics::ShaderTypes::Fragment)))
			{
				EAE6320_ASSERT(false);
				return result;
			}
			{
				
				if (!(result = s_renderState.Initialize(render_state)))
				{
					
					EAE6320_ASSERT(false);
					return result;
				}
			}

			return result;
		}
		eae6320::cResult cEffect::CleanShaderBase()
		{
			auto result = eae6320::Results::Success;
			if (s_vertexShader)
			{
				const auto localResult = cShader::s_manager.Release(s_vertexShader);
				if (!localResult)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = localResult;
					}
				}
			}
			if (s_fragmentShader)
			{
				const auto localResult = cShader::s_manager.Release(s_fragmentShader);
				if (!localResult)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = localResult;
					}
				}
			}

			return result;
		}


		

	}
	eae6320::cResult eae6320::Graphics::cEffect::Load(const std::string& i_path, Graphics::cEffect*& o_effect_data, const Graphics::ShaderTypes::eType i_type)
	{
		auto result = eae6320::Results::Success;


		auto instance = new Graphics::cEffect;
		o_effect_data = instance;


		//Load Binary Asset
		Platform::sDataFromFile pData;
		Platform::LoadBinaryFile(i_path.c_str(), pData);
		uintptr_t start = reinterpret_cast<uintptr_t>(pData.data);
		uintptr_t current = start;

		uint8_t renderState = *reinterpret_cast<uint8_t*>(current);
		current += sizeof(uint8_t);

		uint16_t  pathLen = *reinterpret_cast<uint16_t*>(current);
		current += sizeof(uint16_t);

		auto path = reinterpret_cast<char*>(current);
		current += sizeof(char)*(pathLen);
		auto path1 = reinterpret_cast<char*>(current);
		current += sizeof(char)*(pathLen);

		result = instance->InitializeShadingData(path, path1, renderState);


		return  result;
	}

}
