#pragma once
#include "Engine/Results/cResult.h"


#if defined( EAE6320_PLATFORM_WINDOWS )
#include "Direct3D//Includes.h"
#elif
#include "OpenGL/Includes.h"
#endif
#include "cShader.h"
#include "sContext.h"
#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>

#include <Engine/Platform/Platform.h>
#include <utility>
#include "Tools/ShaderBuilder/cShaderBuilder.h"
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Results/Results.h>

namespace eae6320
{
	namespace Graphics
	{

		class cMesh
		{
		public:
			cMesh();
			~cMesh();

			cResult CleanUp();
			void Draw(bool needSet);
			cResult InitializeGeometry(std::vector<VertexFormats::sMesh> veticesArray, std::vector<uint16_t> indexArray);

			eae6320::cResult LoadAsset(const std::string&  i_path,std::vector<Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);

			// Assets
			//-------

			using Handle = Assets::cHandle<cMesh>;
			static Assets::cManager<cMesh> s_manager;

			static cResult Load(const std::string& i_path, cMesh*& o_mesh_data, const ShaderTypes::eType i_type);

			
			// Geometry Data
			//--------------
#ifdef EAE6320_PLATFORM_D3D
			// A vertex buffer holds the data for each vertex
			ID3D11Buffer* s_vertexBuffer = nullptr;

			// A index buffer holds the data for each vertex
			ID3D11Buffer* s_indexBuffer = nullptr;

			// D3D has an "input layout" object that associates the layout of the vertex format struct
			// with the input from a vertex shader
			ID3D11InputLayout* s_vertexInputLayout = nullptr;
#elif EAE6320_PLATFORM_GL
				// A vertex buffer holds the data for each vertex
			GLuint s_vertexBufferId = 0;	
			// A vertex buffer holds the data for each vertex
			GLuint s_indexBufferId = 0;
			// A vertex array encapsulates the vertex data as well as the vertex input layout
			GLuint s_vertexArrayId = 0;	
		
#endif

			uint16_t index_length;
			void InitializeMesh();
			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cMesh);
		
			static eae6320::cResult MeshFactory(const char* const i_path, Handle &mesh_handle)
			{

				//Load Data
				auto result = eae6320::Results::Success;

				std::vector<VertexFormats::sMesh> veticesArray;
				std::vector<uint16_t> indexArray;
				
				if (!(result = s_manager.Load(i_path,
					mesh_handle, eae6320::Graphics::ShaderTypes::Vertex)))
				{
					result = s_manager.Load("data/Meshes/questionMark.mesh",
						mesh_handle, eae6320::Graphics::ShaderTypes::Vertex);
				}

			
				 return result;
			}
		
		};

	}
}





