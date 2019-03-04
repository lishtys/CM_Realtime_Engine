#include "Engine/Results/cResult.h"
#include "Engine/Results/Results.h"
#include "Tools/ShaderBuilder/cShaderBuilder.h"
#include "cRenderState.h"
#include "cEffect.h"
#include <d3d11.h>
#include "sContext.h"

namespace eae6320
{
	namespace Graphics
	{
		cEffect::cEffect()
		{
		}

		cEffect::~cEffect()
		{
			CleanUp();
		}

		eae6320::cResult cEffect::InitializeShadingData(const char filePath[], const char filePath2[],uint8_t render_State)
		{
			auto result = eae6320::Results::Success;

			result = LoadShadingData(filePath, filePath2, render_State);

			return result;
		}

		eae6320::cResult cEffect::CleanUp()
		{
			return	CleanShaderBase();
		}

		void cEffect::BindShadingData()
		{
			auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
			{
				{
					constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
					constexpr unsigned int interfaceCount = 0;
					// Vertex shader
					{
						EAE6320_ASSERT(s_vertexShader);
						auto* const shader = cShader::s_manager.Get(s_vertexShader);
						EAE6320_ASSERT(shader && shader->m_shaderObject.vertex);
						direct3dImmediateContext->VSSetShader(shader->m_shaderObject.vertex, noInterfaces, interfaceCount);
					}
					// Fragment shader
					{
						EAE6320_ASSERT(s_fragmentShader);
						auto* const shader = cShader::s_manager.Get(s_fragmentShader);
						EAE6320_ASSERT(shader && shader->m_shaderObject.fragment);
						direct3dImmediateContext->PSSetShader(shader->m_shaderObject.fragment, noInterfaces, interfaceCount);
					}
				}
				s_renderState.Bind();
			}

		}


	}
}
