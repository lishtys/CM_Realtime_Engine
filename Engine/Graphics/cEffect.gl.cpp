
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



		eae6320::cResult cEffect::InitializeShadingData(const char filePath[], const char filePath2[],uint8_t renderState)
		{
			auto result = eae6320::Results::Success;



			result = LoadShadingData(filePath, filePath2,renderState);

			// Create a program
			{
				s_programId = glCreateProgram();
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to create a program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
				else if (s_programId == 0)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("OpenGL failed to create a program");
					goto OnExit;
				}
			}
			// Attach the shaders to the program
			{
				// Vertex
				{
					glAttachShader(s_programId, eae6320::Graphics::cShader::s_manager.Get(s_vertexShader)->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the vertex shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						goto OnExit;
					}
				}
				// Fragment
				{
					glAttachShader(s_programId, eae6320::Graphics::cShader::s_manager.Get(s_fragmentShader)->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the fragment shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						goto OnExit;
					}
				}
			}
			// Link the program
			{
				glLinkProgram(s_programId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					// Get link info
					// (this won't be used unless linking fails
					// but it can be useful to look at when debugging)
					std::string linkInfo;
					{
						GLint infoSize;
						glGetProgramiv(s_programId, GL_INFO_LOG_LENGTH, &infoSize);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							struct sLogInfo
							{
								GLchar* memory;
								sLogInfo(const size_t i_size) { memory = reinterpret_cast<GLchar*>(malloc(i_size)); }
								~sLogInfo() { if (memory) free(memory); }
							} info(static_cast<size_t>(infoSize));
							constexpr GLsizei* const dontReturnLength = nullptr;
							glGetProgramInfoLog(s_programId, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
							const auto errorCode = glGetError();
							if (errorCode == GL_NO_ERROR)
							{
								linkInfo = info.memory;
							}
							else
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								eae6320::Logging::OutputError("OpenGL failed to get link info of the program: %s",
									reinterpret_cast<const char*>(gluErrorString(errorCode)));
								goto OnExit;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to get the length of the program link info: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							goto OnExit;
						}
					}
					// Check to see if there were link errors
					GLint didLinkingSucceed;
					{
						glGetProgramiv(s_programId, GL_LINK_STATUS, &didLinkingSucceed);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							if (didLinkingSucceed == GL_FALSE)
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, linkInfo.c_str());
								eae6320::Logging::OutputError("The program failed to link: %s",
									linkInfo.c_str());
								goto OnExit;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to find out if linking of the program succeeded: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							goto OnExit;
						}
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to link the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}

		OnExit:

			if (!result)
			{
				if (s_programId != 0)
				{
					glDeleteProgram(s_programId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
					}
					s_programId = 0;
				}
			}

			return result;

		}

		eae6320::cResult cEffect::CleanUp()
		{
			auto result = eae6320::Results::Success;
			result=	CleanShaderBase();
		
			if (s_programId != 0)
			{
				glDeleteProgram(s_programId);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = eae6320::Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				s_programId = 0;
			}

			{
				const auto localResult = s_renderState.CleanUp();
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

		void cEffect::BindShadingData()
		{
			{
				{
					EAE6320_ASSERT(s_programId != 0);
					glUseProgram(s_programId);
					EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
				}
				s_renderState.Bind();
			}
		}




	}
}
