

 #include "Graphics.h"

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
 #include "Engine/Graphics/cMesh.h"
 #include "Engine/Graphics/cEffect.h"
#include "Engine/Graphics/GraphicBase.h"
#include "Engine/Graphics/Color.h"
#include "sRenderCommand.h"
#include "Tools/MeshBuilder/cMeshBuilder.h"
#include <algorithm>
#include "cMaterial.h"
#include "cSamplerState.h"

// Static Data Initialization
 //===========================
 namespace
 {
 	// // In Direct3D "views" are objects that allow a texture to be used a particular way:
 	// // A render target view allows a texture to have color rendered to it
 	// ID3D11RenderTargetView* s_renderTargetView = nullptr;
 	// // A depth/stencil view allows a texture to have depth rendered to it
 	// ID3D11DepthStencilView* s_depthStencilView = nullptr;
  
 	 // Constant buffer object
 	 eae6320::Graphics::cConstantBuffer s_constantBuffer_perFrame(eae6320::Graphics::ConstantBufferTypes::PerFrame);
 	 eae6320::Graphics::cConstantBuffer s_constantBuffer_perDrawCall(eae6320::Graphics::ConstantBufferTypes::PerDrawCall);
 	 eae6320::Graphics::cConstantBuffer s_constantBuffer_perMaterial(eae6320::Graphics::ConstantBufferTypes::PerMaterial);
  
	 eae6320::Graphics::cSamplerState::Handle s_sampler;


 	// Submission Data
 	//----------------

 	// This struct's data is populated at submission time;
 	// it must cache whatever is necessary in order to render a frame
 	struct sDataRequiredToRenderAFrame
 	{
 		eae6320::Graphics::ConstantBufferFormats::sPerFrame constantData_perFrame;
 		eae6320::Graphics::ConstantBufferFormats::sPerDrawCall constantData_perDrawcall;
 		eae6320::Graphics::ConstantBufferFormats::sPerMaterial constantData_perMtaterial;
		eae6320::Graphics::Color backgroundColor;
		eae6320::Graphics::cMesh** meshArray;
		eae6320::Graphics::cMaterial** matArray;
		eae6320::Math::cQuaternion* quaternionArray;
		eae6320::Math::sVector* positionArray;


		std::vector<eae6320::Graphics::Commands::sRenderCmd> cmds;
		uint_fast32_t* mesh_handle_array;

		uint16_t i_meshSize;

 	};
 	// In our class there will be two copies of the data required to render a frame:
 	//	* One of them will be getting populated by the data currently being submitted by the application loop thread
 	//	* One of them will be fully populated, 
 	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
 	auto* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
 	auto* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
 	// The following two events work together to make sure that
 	// the main/render thread and the application loop thread can work in parallel but stay in sync:
 	// This event is signaled by the application loop thread when it has finished submitting render data for a frame
 	// (the main/render thread waits for the signal)
 	eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
 	// This event is signaled by the main/render thread when it has swapped render data pointers.
 	// This means that the renderer is now working with all the submitted data it needs to render the next frame,
 	// and the application loop thread can start submitting data for the following frame
 	// (the application loop thread waits for the signal)
 	eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;

 	// Shading Data
 	//-------------
 	eae6320::Graphics::cEffect s_effect_instance;
 	eae6320::Graphics::cEffect s_effect_instance1;


 	// Geometry Data
 	//--------------
 	eae6320::Graphics::cMesh s_mesh_instance;
 	eae6320::Graphics::cMesh s_mesh_instance1;

 	// A vertex buffer holds the data for each vertex
 	ID3D11Buffer* s_vertexBuffer = nullptr;
 	// D3D has an "input layout" object that associates the layout of the vertex format struct
 	// with the input from a vertex shader
 	ID3D11InputLayout* s_vertexInputLayout = nullptr;
 }





 // Interface
 //==========

 // Submission
 //-----------

 void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
 {
 	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
 	auto& constantData_perFrame = s_dataBeingSubmittedByApplicationThread->constantData_perFrame;
 	constantData_perFrame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
 	constantData_perFrame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
 }

 eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
 {
 	return Concurrency::WaitForEvent(s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
 }

 eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
 {
 	return s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
 }
 
void eae6320::Graphics::SubmitClearColor(Color color)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	s_dataBeingSubmittedByApplicationThread->backgroundColor = color;
}

void eae6320::Graphics::SubmitCameraInfo(Math::cMatrix_transformation i_worldToCamera, Math::cMatrix_transformation i_cameraToProjected,Math::sVector camPos)
{
	s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_transform_worldToCamera = i_worldToCamera;
	s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_transform_cameraToProjected = i_cameraToProjected;
	s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_cameraPosition_world = camPos;

}

void eae6320::Graphics::SubmitLightInfo(Math::sVector lightRot, Color lightColor)
{
	s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_lightColor = lightColor;
	s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_light_Rot = lightRot;
}


void eae6320::Graphics::SubmitMeshEffectPairs(uint_fast32_t meshIdx, uint_fast32_t matIdx, uint_fast32_t effectIdx,cMesh *meshInstance, cMaterial *matInstance,Math::cQuaternion rotation,Math::sVector position)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& meshArray = s_dataBeingSubmittedByApplicationThread->meshArray;
	auto& matArray = s_dataBeingSubmittedByApplicationThread->matArray;
	auto& quaternionArray = s_dataBeingSubmittedByApplicationThread->quaternionArray;
	auto& positionArray = s_dataBeingSubmittedByApplicationThread->positionArray;
    auto index=	s_dataBeingSubmittedByApplicationThread->i_meshSize;
	meshArray[index] = meshInstance;
	matArray[index] = matInstance;
	quaternionArray[index] = rotation;
	positionArray[index] = position;
	meshInstance->IncrementReferenceCount();
	matInstance->IncrementReferenceCount();
	s_dataBeingSubmittedByApplicationThread->i_meshSize++;



	



	auto& meshHandleArray = s_dataBeingSubmittedByApplicationThread->mesh_handle_array;

	meshHandleArray[index] = meshIdx;

	auto& cmds = s_dataBeingSubmittedByApplicationThread->cmds;

	Commands::sRenderCmd render_cmd;


;
	auto iDependent= cEffect::s_manager.UnsafeGet(effectIdx)->s_renderState.IsAlphaTransparencyEnabled()?1:0;
	 // auto iDependent=	1;

	
	auto zVal = (s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_transform_worldToCamera * Math::cMatrix_transformation(rotation, position)).GetTranslation().z;
	if (zVal >= 0) zVal = 0;
	zVal = round((-zVal - 0.3f) / (1000 - 0.3f) * 1024);

	if (iDependent) zVal = 1024 - zVal;
	if (!iDependent)
	{
		render_cmd.Val = static_cast<uint64_t>(iDependent) << 63;  // 64
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(effectIdx) << 55);  // 56-63
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(matIdx) << 47);  // 48-55
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(zVal) << 37);  // 38-47
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(meshIdx) << 8); //9-16
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(index)); //1-8
	}
	else
	{
		render_cmd.Val = static_cast<uint64_t>(iDependent) << 63;  // 64
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(zVal) << 53);  // 54-63
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(effectIdx) << 45);  // 46-53
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(matIdx) << 37);  // 38-45
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(meshIdx) << 8); //9-16
		render_cmd.Val = (render_cmd.Val) | (static_cast<uint64_t>(index)); //1-8
	}

	cmds.push_back(render_cmd);
}


// Render
 //-------

 void eae6320::Graphics::RenderFrame()
 {

 	// Wait for the application loop to submit data to be rendered
 	{
 		const auto result = Concurrency::WaitForEvent(s_whenAllDataHasBeenSubmittedFromApplicationThread);
 		if (result)
 		{
 			// Switch the render data pointers so that
 			// the data that the application just submitted becomes the data that will now be rendered
 			std::swap(s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread);
 			// Once the pointers have been swapped the application loop can submit new data
 			const auto result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
 			if (!result)
 			{
 				EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
 				Logging::OutputError("Failed to signal that new render data can be submitted");
 				UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
 					" The application is probably in a bad state and should be exited");
 				return;
 			}
 		}
 		else
 		{
 			EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
 			Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
 			UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
 				" The application is probably in a bad state and should be exited");
 			return;
 		}
 	}

	


	EAE6320_ASSERT(s_dataBeingRenderedByRenderThread);
 	// Update the per-frame constant buffer
 	{
 		// Copy the data from the system memory that the application owns to GPU memory
 		auto& constantData_perFrame = s_dataBeingRenderedByRenderThread->constantData_perFrame;
 		s_constantBuffer_perFrame.Update(&constantData_perFrame);
 	} 
 	




	// auto stime = sin(s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_elapsedSecondCount_simulationTime);
	// auto ctime = cos(s_dataBeingSubmittedByApplicationThread->constantData_perFrame.g_elapsedSecondCount_simulationTime);
	// Clear Color

	GraphicsBase::ClearTarget(s_dataBeingRenderedByRenderThread->backgroundColor);

	{
	  auto matArray=	s_dataBeingRenderedByRenderThread->matArray;
	  auto meshArray=s_dataBeingRenderedByRenderThread->meshArray;
	  auto quaternionArray =s_dataBeingRenderedByRenderThread->quaternionArray;
	  auto positionArray=s_dataBeingRenderedByRenderThread->positionArray;

	  auto meshCnt = s_dataBeingRenderedByRenderThread->i_meshSize;;

	  auto handleArray = s_dataBeingRenderedByRenderThread->mesh_handle_array;

	  auto cmds = s_dataBeingRenderedByRenderThread->cmds;


	  std::sort(cmds.begin(), cmds.end(), [](Commands::sRenderCmd a, Commands::sRenderCmd b)
	  {
		  
		  return a.Val < b.Val;
	  } );




	  uint_fast32_t lastEffects = 100000;
	  uint_fast32_t lastMesh = 100000;
	  uint_fast32_t lastMat = 100000;
		for (int i = 0; i <meshCnt; ++i)
		{

			auto cmd = cmds[i];
			
			auto meshIdx = static_cast<uint_fast32_t> (7 & cmd.Val);
			//  Update the per-Drawcall constant buffer
			{
				// Copy the data from the system memory that the application owns to GPU memory
				auto& constantData_perDrawcall = s_dataBeingRenderedByRenderThread->constantData_perDrawcall;
				constantData_perDrawcall.g_transform_localToWorld = Math::cMatrix_transformation(quaternionArray[meshIdx], positionArray[meshIdx]);
				constantData_perDrawcall.g_transform_localToProject = s_dataBeingRenderedByRenderThread->constantData_perFrame.g_transform_cameraToProjected* s_dataBeingRenderedByRenderThread->constantData_perFrame.g_transform_worldToCamera*constantData_perDrawcall.g_transform_localToWorld;
				s_constantBuffer_perDrawCall.Update(&constantData_perDrawcall);

			
			}

			auto idepend = static_cast<uint_fast32_t> (7 & (cmd.Val >> 63));
			uint_fast32_t effectIdx = 0;
			uint_fast32_t matIdx = 0;
			uint_fast32_t meshHandleIdx = 0;

		
			
			if (!idepend)
			{
				effectIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 55));
				matIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 47));
				meshHandleIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 8));
			}
			else
			{
				effectIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 45));
				matIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 37));
				meshHandleIdx = static_cast<uint_fast32_t> (7 & (cmd.Val >> 8));
			}


			


			if(lastEffects!= effectIdx)
			{
				cEffect::s_manager.UnsafeGet(effectIdx)->BindShadingData();
				lastEffects = effectIdx;
			}

		


			if (lastMat != matIdx)
			{
				auto& constantData_perMaterial = s_dataBeingRenderedByRenderThread->constantData_perMtaterial;
				auto matInstance = cMaterial::s_manager.UnsafeGet(matIdx);
				constantData_perMaterial.g_tintColor = matInstance->m_color;
				s_constantBuffer_perMaterial.Update(&constantData_perMaterial);

				auto textInstance = cTexture::s_manager.Get(matInstance->m_texture_handle);
				textInstance->Bind(0);
				lastMat = matIdx;



			}

			//Whether Set Buffer
			bool needNext = lastMesh != meshHandleIdx;

			if(needNext)
			{
				lastMesh = meshHandleIdx;
			}



			cMesh::s_manager.UnsafeGet(meshHandleIdx)->Draw(needNext);
		
		}

	}

	GraphicsBase::Swap();

 	
 	// Once everything has been drawn the data that was submitted for this frame
 	// should be cleaned up and cleared.
 	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
 	{
		auto matArray = s_dataBeingRenderedByRenderThread->matArray;
		auto meshArray = s_dataBeingRenderedByRenderThread->meshArray;
		auto quaternionArray = s_dataBeingRenderedByRenderThread->quaternionArray;
		auto positionArray = s_dataBeingRenderedByRenderThread->positionArray;
		auto meshCnt = s_dataBeingRenderedByRenderThread->i_meshSize;;
		s_dataBeingRenderedByRenderThread->cmds.clear();
 		// (At this point in the class there isn't anything that needs to be cleaned up)
		for (int i = 0; i <meshCnt; ++i)
		{
			matArray[i]->DecrementReferenceCount();
			meshArray[i]->DecrementReferenceCount();
			matArray[i] = nullptr;
			meshArray[i] = nullptr;
			s_dataBeingRenderedByRenderThread->i_meshSize--;

		}

 #pragma endregion
 	}

 }

 // Initialization / Clean Up
 //--------------------------

 eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
 {
 	auto result = Results::Success;

 	// Initialize the platform-specific context
 	if (!(result = Graphics::sContext::g_context.Initialize(i_initializationParameters)))
 	{
 		EAE6320_ASSERT(false);
 		goto OnExit;
 	}
 	// Initialize the asset managers
 	{
 		if (!(result = Graphics::cShader::s_manager.Initialize()))
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 	}
	
 	// Initialize the platform-independent graphics objects
 	{
 		if (result = s_constantBuffer_perFrame.Initialize())
 		{
 			// There is only a single per-frame constant buffer that is re-used
 			// and so it can be bound at initialization time and never unbound
 			s_constantBuffer_perFrame.Bind(
 				// In our class both vertex and fragment shaders use per-frame constant data
 				ShaderTypes::Vertex | ShaderTypes::Fragment);
 		}
 		else
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 	}

	// Initialize Draw Call
 	{
 		if (result = s_constantBuffer_perDrawCall.Initialize())
 		{
 			//todo  dawCall Initialize
			s_constantBuffer_perDrawCall.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment);
 		}
 		else
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 	}


	// Initialize Material 
	{
		if (result = s_constantBuffer_perMaterial.Initialize())
		{
			//todo  dawCall Initialize
			s_constantBuffer_perMaterial.Bind(
				ShaderTypes::Vertex | ShaderTypes::Fragment);
		}
		else
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
	}
	
	uint8_t sampler_states;
	sampler_states = SamplerStates::eSamplerState::Tile;
	SamplerStates::EnableFiltering(sampler_states);
	SamplerStates::SetEdgeBehaviorU(SamplerStates::eSamplerState::Tile,sampler_states);
	SamplerStates::SetEdgeBehaviorV(SamplerStates::eSamplerState::Tile,sampler_states);

	if (result = Graphics::cSamplerState::s_manager.Load(sampler_states,s_sampler))
	{
		
		cSamplerState::s_manager.Get(s_sampler)->Bind(0);
	}



 	// Initialize the events
 	{
 		if (!(result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 		if (!(result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
 			Concurrency::EventState::Signaled)))
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 	}
 	// Initialize the views
 	{
 		if (!(result = GraphicsBase::Initialize(i_initializationParameters)))
 		{
 			EAE6320_ASSERT(false);
 			goto OnExit;
 		}
 	}

	 //Initialize Data Buffers
 	{
		s_dataBeingSubmittedByApplicationThread->meshArray = new cMesh*[100];
		s_dataBeingSubmittedByApplicationThread->matArray = new cMaterial*[100];
		s_dataBeingSubmittedByApplicationThread->positionArray = new Math::sVector[100];
		s_dataBeingSubmittedByApplicationThread->quaternionArray = new Math::cQuaternion[100];
		s_dataBeingSubmittedByApplicationThread->i_meshSize = 0;
		s_dataBeingSubmittedByApplicationThread->mesh_handle_array = new uint_fast32_t[100];

 	}
 	{
		s_dataBeingRenderedByRenderThread->meshArray = new cMesh*[100];
		s_dataBeingRenderedByRenderThread->matArray = new cMaterial*[100];
		s_dataBeingRenderedByRenderThread->positionArray = new Math::sVector[100];
		s_dataBeingRenderedByRenderThread->quaternionArray = new Math::cQuaternion[100];
		s_dataBeingRenderedByRenderThread->i_meshSize = 0;
		s_dataBeingRenderedByRenderThread->mesh_handle_array = new uint_fast32_t[100];
 	}




 OnExit:

 	return result;
 }

 eae6320::cResult eae6320::Graphics::CleanUp()
 {
	 auto result = Results::Success;



	 eae6320::GraphicsBase::CleanUp();



	 auto matArray = s_dataBeingRenderedByRenderThread->matArray;
	 auto meshArray = s_dataBeingRenderedByRenderThread->meshArray;
	 auto quaternionArray = s_dataBeingRenderedByRenderThread->quaternionArray;
	 auto positionArray = s_dataBeingRenderedByRenderThread->positionArray;
	 auto isize = s_dataBeingRenderedByRenderThread->i_meshSize;

	 auto meshHandleArray = s_dataBeingRenderedByRenderThread->mesh_handle_array;

	 // (At this point in the class there isn't anything that needs to be cleaned up)
	 for (int i = 0; i <isize; ++i)
	 {
		 matArray[i]->DecrementReferenceCount();
		 meshArray[i]->DecrementReferenceCount();
		 meshArray[i] = nullptr;
		 matArray[i] = nullptr;
		 s_dataBeingRenderedByRenderThread->i_meshSize--;
	 }

	 delete[] matArray;
	 delete[] meshArray;
	 delete[] quaternionArray;
	 delete[] positionArray;
	 delete[] meshHandleArray;


	 matArray = s_dataBeingSubmittedByApplicationThread->matArray;
	  meshArray = s_dataBeingSubmittedByApplicationThread->meshArray;
	  quaternionArray = s_dataBeingSubmittedByApplicationThread->quaternionArray;
	  positionArray = s_dataBeingSubmittedByApplicationThread->positionArray;
	  isize = s_dataBeingSubmittedByApplicationThread->i_meshSize;
	
 	meshHandleArray = s_dataBeingSubmittedByApplicationThread->mesh_handle_array;
	 
 	for (int i = 0; i <isize; ++i)
	  {
		matArray[i]->DecrementReferenceCount();
		  meshArray[i]->DecrementReferenceCount();
		  meshArray[i] = nullptr;
		  matArray[i] = nullptr;
		  s_dataBeingSubmittedByApplicationThread->i_meshSize--;
	  }

	  delete[] matArray;
	  delete[] meshArray;
	  delete[] quaternionArray;
	  delete[] positionArray;
	  delete[] meshHandleArray;
 		
 		{
 			const auto localResult = s_constantBuffer_perFrame.CleanUp();
 			if (!localResult)
 			{
 				EAE6320_ASSERT(false);
 				if (result)
 				{
 					result = localResult;
 				}
 			}
 		}	
 		
 		{
 			const auto localResult = s_constantBuffer_perMaterial.CleanUp();
 			if (!localResult)
 			{
 				EAE6320_ASSERT(false);
 				if (result)
 				{
 					result = localResult;
 				}
 			}
 		}
	
 	//PerDrawCall  CleanUp
 		{
 			const auto localResult = s_constantBuffer_perDrawCall.CleanUp();
 			if (!localResult)
 			{
 				EAE6320_ASSERT(false);
 				if (result)
 				{
 					result = localResult;
 				}
 			}
 		}

 		{
 			const auto localResult = Graphics::cShader::s_manager.CleanUp();
 			if (!localResult)
 			{
 				EAE6320_ASSERT(false);
 				if (result)
 				{
 					result = localResult;
 				}
 			}
 		}


	 // Sampler State Clean
		if (s_sampler)
		{
			const auto localResult = cSamplerState::s_manager.Release(s_sampler);
			if (!localResult)
			{
				EAE6320_ASSERT(false);
				if (result)
				{
					result = localResult;
				}
			}
		}


		{
			const auto localResult = Graphics::cSamplerState::s_manager.CleanUp();
			if (!localResult)
			{
				EAE6320_ASSERT(false);
				if (result)
				{
					result = localResult;
				}
			}
		}


 		{
 			const auto localResult = Graphics::sContext::g_context.CleanUp();
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



