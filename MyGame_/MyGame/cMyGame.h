/*
	This class is your specific game
*/
#pragma once
#ifndef EAE6320_CEXAMPLEGAME_H
#define EAE6320_CEXAMPLEGAME_H

// Includes
//=========

#include <Engine/Application/cbApplication.h>
#include <Engine/Results/Results.h>
#include "Engine/Graphics/Color.h"
#include "Engine/UserInput/UserInput.h"
#include "Engine/Physics/sRigidBodyState.h"
#include "Engine/Graphics/cCamera.h"
#include "Engine/Graphics/GraphicBase.h"
#include "Engine/Graphics/cGameObject.h"
#include "Engine/Graphics/cMaterial.h"


#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "Resource Files/Resource.h"
#endif

// Class Declaration
//==================

namespace eae6320
{
	class cMyGame : public Application::cbApplication
	{
		// Inherited Implementation
		//=========================

	private:

		// Configuration
		//--------------

#if defined( EAE6320_PLATFORM_WINDOWS )
		// The main window's name will be displayed as its caption (the text that is displayed in the title bar).
		// You can make it anything that you want, but please keep the platform name and debug configuration at the end
		// so that it's easy to tell at a glance what kind of build is running.
		virtual const char* GetMainWindowName() const override
		{
			return "Chen Mi's EAE6320 Example Game"
				" -- "
#if defined( EAE6320_PLATFORM_D3D )
				"Direct3D"
#elif defined( EAE6320_PLATFORM_GL )
				"OpenGL"
#endif
#ifdef _DEBUG
				" -- Debug"
#endif
				;
		}
		// Window classes are almost always identified by name;
		// there is a unique "ATOM" associated with them,
		// but in practice Windows expects to use the class name as an identifier.
		// If you don't change the name below
		// your program could conceivably have problems if it were run at the same time on the same computer
		// as one of your classmate's.
		// You don't need to worry about this for our class,
		// but if you ever ship a real project using this code as a base you should set this to something unique
		// (a generated GUID would be fine since this string is never seen)
		virtual const char* GetMainWindowClassName() const override { return "Chen Mi's EAE6320 Example Main Window Class"; }
		// The following three icons are provided:
		//	* IDI_EAEGAMEPAD
		//	* IDI_EAEALIEN
		//	* IDI_VSDEFAULT_LARGE / IDI_VSDEFAULT_SMALL
		// If you want to try creating your own a convenient website that will help is: http://icoconvert.com/
		virtual const WORD* GetLargeIconId() const override { static constexpr WORD iconId_large = IDI_LIYUWANG; return &iconId_large; }
		virtual const WORD* GetSmallIconId() const override { static constexpr WORD iconId_small = IDI_LIYUWANG; return &iconId_small; }
#endif

		// Run
		//----

		virtual void UpdateBasedOnInput() override;
		virtual void UpdateSimulationBasedOnInput() override;
		virtual void UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) override;



	

		// Initialization / Clean Up
		//--------------------------

		virtual cResult Initialize() override;
		virtual cResult CleanUp() override;

		virtual void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime,
			const float i_elapsedSecondCount_sinceLastSimulationUpdate) override
			;


	private:
		bool isHide=false;
		bool notAnimated=false;
		bool autoHide=false;

		float time_count=0;

		bool isLeft=false;
		bool isRight=false;
		bool isForward=false;
		bool isBack=false;
		bool isRotLeft=false;
		bool isRotRight =false;
		bool isRotDown =false;
		bool isRotUp =false;


		bool isGoLeft = false;
		bool isGoRight = false;
		bool isGoUp = false;
		bool isGoDown = false;



		bool isLightRotLeft = false;
		bool isLightRotRight = false;
		bool isLightRotUp = false;
		bool isLightRotDown = false;

	

		//UserCtrl
		void UpdateUserCtrl();
		

		void UpdateGoMovement(const float i_elapsedSecondCount_sinceLastUpdate,Graphics::cGameObject *selectedGameobject);
		void UpdateCameraMovement(const float i_elapsedSecondCount_sinceLastUpdate,Graphics::cGameObject *selectedGameobject);
		void UpdateLightMovement(const float i_elapsedSecondCount_sinceLastUpdate);
		

		// Initial Camera Matrix

		void SubmitCameraInfo(Graphics::cCamera renderedCamera, const float i_elapsedSecondCount_sinceLastUpdate)
		{
		  
			renderedCamera.SubmitCameraInfo(i_elapsedSecondCount_sinceLastUpdate);
		}


	};
}

// Result Definitions
//===================

namespace eae6320
{
	namespace Results
	{
		namespace Application
		{
			// You can add specific results for your game here:
			//	* The System should always be Application
			//	* The __LINE__ macro is used to make sure that every result has a unique ID.
			//		That means, however, that all results _must_ be defined in this single file
			//		or else you could have two different ones with equal IDs.
			//	* Note that you can define multiple Success codes.
			//		This can be used if the caller may want to know more about how a function succeeded.
			constexpr cResult ExampleResult( IsFailure, System::Application, __LINE__, Severity::Default );
		}
	}
}

#endif	// EAE6320_CEXAMPLEGAME_H
