#include "cMesh.h"
#include <iostream>

namespace eae6320
{
	namespace Graphics
	{
		cMesh::cMesh()
		{
		}

		cMesh::~cMesh()
		{
			CleanUp();
		}

		eae6320::cResult cMesh::CleanUp()
		{

			if (s_indexBuffer)
			{
				s_indexBuffer->Release();
				s_indexBuffer = nullptr;
			}

			if (s_vertexBuffer)
			{
				s_vertexBuffer->Release();
				s_vertexBuffer = nullptr;
			}
			if (s_vertexInputLayout)
			{
				s_vertexInputLayout->Release();
				s_vertexInputLayout = nullptr;
			}

			return Results::Success;
		}

		void cMesh::Draw(bool needSet)
		{

			auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;



			// Bind a specific vertex buffer to the device as a data source
			{
				EAE6320_ASSERT(s_vertexBuffer);
				constexpr unsigned int startingSlot = 0;
				constexpr unsigned int vertexBufferCount = 1;
				// The "stride" defines how large a single vertex is in the stream of data
				constexpr unsigned int bufferStride = sizeof(VertexFormats::sMesh);
				// It's possible to start streaming data in the middle of a vertex buffer
				constexpr unsigned int bufferOffset = 0;
				
				 if(needSet)
				direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &s_vertexBuffer, &bufferStride, &bufferOffset);
			}


			// Bind a specific index buffer to the device as a data source
			{

				EAE6320_ASSERT(s_indexBuffer);
				constexpr DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
				// The indices start at the beginning of the buffer
				constexpr unsigned int offset = 0;

				if (needSet)
				direct3dImmediateContext->IASetIndexBuffer(s_indexBuffer, indexFormat, offset);

			}


			// Specify what kind of data the vertex buffer holds
			{
				// Set the layout (which defines how to interpret a single vertex)
				{
					EAE6320_ASSERT(s_vertexInputLayout);
					 if (needSet)
					direct3dImmediateContext->IASetInputLayout(s_vertexInputLayout);
				
				}
				// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
				// the vertex buffer was defined as a triangle list
				// (meaning that every primitive is a triangle and will be defined by three vertices)
	
			 //  	direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
			}
			// Render triangles from the currently-bound vertex buffer
			{
				// As of this comment only a single triangle is drawn
				// (you will have to update this code in future assignments!)
			
				// It's possible to start rendering primitives in the middle of the stream

				constexpr unsigned int indexOfFirstIndexToUse = 0;
				constexpr unsigned int offsetToAddToEachIndex = 0;
				direct3dImmediateContext->DrawIndexed(static_cast<unsigned int>(index_length), indexOfFirstIndexToUse, offsetToAddToEachIndex);
			}

		}

		
		eae6320::cResult cMesh::InitializeGeometry(std::vector<VertexFormats::sMesh> veticesArray, std::vector<uint16_t> indexArray)
		{
			std::reverse(indexArray.begin(), indexArray.end());
			auto result = eae6320::Results::Success;
			index_length =  (uint16_t)indexArray.size();;
			
			

			auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
			EAE6320_ASSERT(direct3dDevice);

			// Initialize vertex format
			{
				// Load the compiled binary vertex shader for the input layout
				eae6320::Platform::sDataFromFile vertexShaderDataFromFile;
				std::string errorMessage;
				if (result = eae6320::Platform::LoadBinaryFile("data/Shaders/Vertex/vertexInputLayout.shader", vertexShaderDataFromFile, &errorMessage))
				{
					// Create the vertex layout

					// These elements must match the VertexFormats::sMesh layout struct exactly.
					// They instruct Direct3D how to match the binary data in the vertex buffer
					// to the input elements in a vertex shader
					// (by using so-called "semantic" names so that, for example,
					// "POSITION" here matches with "POSITION" in shader code).
					// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
					constexpr unsigned int vertexElementCount = 4;
					D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = {};
					{
						// Slot 0

						// POSITION
						// 3 floats == 12 bytes
						// Offset = 0
						{
							auto& positionElement = layoutDescription[0];

							positionElement.SemanticName = "POSITION";
							positionElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
							positionElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
							positionElement.InputSlot = 0;
							positionElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sMesh, x);
							positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
							positionElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
						}
						
						// COLOR
						// 4 uint8 == 32 bytes
						// Offset = 0
						{
							auto& colorElement = layoutDescription[1];

							colorElement.SemanticName = "COLOR";
							colorElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
							colorElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
							colorElement.InputSlot = 0;
							colorElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sMesh, r);
							colorElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
							colorElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
						}
						
						// Text
						// 2 uint8 == 32 bytes
						// Offset = 0
						{
							auto& textureElement = layoutDescription[2];
						
							textureElement.SemanticName = "TEXCOORD";
							textureElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
							textureElement.Format = DXGI_FORMAT_R32G32_FLOAT;
							textureElement.InputSlot = 0;
							textureElement.AlignedByteOffset = 16;
							textureElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
							textureElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
						}
						
						{
							auto& textureElement = layoutDescription[3];
						
							textureElement.SemanticName = "NORMAL";
							textureElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
							textureElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
							textureElement.InputSlot = 0;
							textureElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sMesh, nx);
							textureElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
							textureElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
						}
					}

					const auto d3dResult = direct3dDevice->CreateInputLayout(layoutDescription, vertexElementCount,
						vertexShaderDataFromFile.data, vertexShaderDataFromFile.size, &s_vertexInputLayout);
					if (FAILED(result))
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, "Geometry vertex input layout creation failed (HRESULT %#010x)", d3dResult);
						eae6320::Logging::OutputError("Direct3D failed to create the geometry vertex input layout (HRESULT %#010x)", d3dResult);
					}

					vertexShaderDataFromFile.Free();
				}
				else
				{
					EAE6320_ASSERTF(false, errorMessage.c_str());
					eae6320::Logging::OutputError("The geometry vertex input layout shader couldn't be loaded: %s", errorMessage.c_str());
					goto OnExit;
				}
			}
		
			
			// Vertex Buffer

			{

				D3D11_BUFFER_DESC bufferDescription{};
				{
					const auto bufferSize = veticesArray.size() * sizeof(eae6320::Graphics::VertexFormats::sMesh);
					EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
					bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
					bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
					bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
					bufferDescription.MiscFlags = 0;
					bufferDescription.StructureByteStride = 0;	// Not used
				}
				D3D11_SUBRESOURCE_DATA initialData{};
				{

					initialData.pSysMem = &veticesArray[0];
					// (The other data members are ignored for non-texture buffers)
				}

				const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &s_vertexBuffer);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Geometry vertex buffer creation failed (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create a geometry vertex buffer (HRESULT %#010x)", d3dResult);
					goto OnExit;
				}
			}


			//IndexBuffer
			{


				D3D11_BUFFER_DESC bufferDescription{};
				{
					const auto bufferSize = indexArray.size() * sizeof(uint16_t);
					EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
					bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
					bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
					bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
					bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
					bufferDescription.MiscFlags = 0;
					bufferDescription.StructureByteStride = 0;	// Not used
				}
				D3D11_SUBRESOURCE_DATA initialData{};
				{

					initialData.pSysMem = &indexArray[0];
					// (The other data members are ignored for non-texture buffers)
				}

				const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &s_indexBuffer);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Geometry index buffer creation failed (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create a geometry index buffer (HRESULT %#010x)", d3dResult);
					goto OnExit;
				}
			}


		OnExit:

			return result;

		}

	
	}
}


