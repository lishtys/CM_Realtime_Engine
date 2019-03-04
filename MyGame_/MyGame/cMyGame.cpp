// Includes
//=========
#pragma once
#include "cMyGame.h"
#include <Engine/Asserts/Asserts.h>
#include "Engine/Graphics/cEffect.h"
#include "Engine/Graphics/cMesh.h"
#include "Engine/Graphics/cGameObject.h"
#include "Engine/Assets/cHandle.h"
#include <ctime>
#include "Engine/Graphics/cDirectionLight.h"


//PrefabData
//todo Data Driven In The Future



// Shading Data
//-------------
eae6320::Graphics::cMaterial::Handle s_TexMatAynami_Mat;
eae6320::Graphics::cMaterial::Handle s_TexMatEarth_Mat;
eae6320::Graphics::cMaterial::Handle s_TexMatRem_Mat;
eae6320::Graphics::cMaterial::Handle s_TexMatYo_Mat;
eae6320::Graphics::cMaterial::Handle s_TexMatSuit_Mat;
eae6320::Graphics::cMaterial::Handle s_River_Mat;

std::vector<eae6320::Graphics::cMaterial::Handle> mat_List;




// Geometry Data
//--------------
eae6320::Graphics::cMesh::Handle s_mesh_instance;
 eae6320::Graphics::cMesh::Handle s_mesh_River;
 eae6320::Graphics::cMesh::Handle s_mesh_Sphere;
std::vector<eae6320::Graphics::cMesh::Handle> meshList;

// eae6320::Graphics::cMesh::Handle s_mesh_instance2;

//GameObject Data
eae6320::Graphics::cGameObject s_game_object1;

std::vector<eae6320::Graphics::cGameObject> Teapots;
eae6320::Graphics::cGameObject*s_movable_game_object;




// Camera Data
//--------------
eae6320::Graphics::cCamera s_camera_instance;



//Light Data

cDirectionLight s_direction_light;



// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space))
	{
		Application::cbApplication::SetSimulationRate(.5f);
	}else
	{
		Application::cbApplication::SetSimulationRate(1.0f);
	}

	
}

void eae6320::cMyGame::UpdateSimulationBasedOnInput()
{
	UpdateUserCtrl();
}

void eae6320::cMyGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
	UpdateGoMovement(i_elapsedSecondCount_sinceLastUpdate,s_movable_game_object);
	UpdateCameraMovement(i_elapsedSecondCount_sinceLastUpdate,s_movable_game_object);
	UpdateLightMovement(i_elapsedSecondCount_sinceLastUpdate);
}



// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cMyGame::Initialize()
{


	auto result = Results::Success;
	
	s_direction_light.lightColor = { 1,1,1,1 };


#pragma region Load Mesh Prefab

	{

		if (!(result = Graphics::cMesh::MeshFactory("data/Meshes/Cube.bin",s_mesh_instance)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
		if (!(result = Graphics::cMesh::MeshFactory("data/Meshes/Plane.bin",s_mesh_River)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
if (!(result = Graphics::cMesh::MeshFactory("data/Meshes/Sphere.bin",s_mesh_Sphere)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}


		meshList.push_back(s_mesh_instance);
		meshList.push_back(s_mesh_River);
		meshList.push_back(s_mesh_Sphere);

	}





#pragma endregion

	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/TexEarth.material", s_TexMatEarth_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}
	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/TexMatAynami.material", s_TexMatAynami_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}

	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/TexMatRem.material", s_TexMatRem_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}	
	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/TexMatYo.material", s_TexMatYo_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}
	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/TexMatSuits.material", s_TexMatSuit_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}

	if (!(result = Graphics::cMaterial::MaterialFactory("data/Materials/RiverAnim.material", s_River_Mat)))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}

	mat_List.push_back(s_TexMatEarth_Mat);
	mat_List.push_back(s_TexMatAynami_Mat);
	mat_List.push_back(s_TexMatRem_Mat);
	mat_List.push_back(s_TexMatYo_Mat);
	mat_List.push_back(s_TexMatSuit_Mat);
	mat_List.push_back(s_River_Mat);

	//Initialize game objects
	srand(static_cast<unsigned>(time(NULL)));

	for (int i=0;i<1;i++)
	{
		Graphics::cGameObject teapot;
		int idx = rand() % mat_List.size();
		teapot.LoadRenderInfo(s_mesh_Sphere, s_TexMatEarth_Mat);
		Math::sVector pos;
		pos.x = pos.x + i / 2 * 1.3f - 1;
		pos.y = pos.y + i%2*1.3f-1;
		pos.z = 5;
		teapot.s_rigid_body_state.position = pos;
		Teapots.push_back(teapot);
	}
	

	{
		Graphics::cGameObject teapot;
		int idx = rand() % mat_List.size();
		teapot.LoadRenderInfo(s_mesh_River, s_River_Mat);
		Math::sVector pos;
		pos.x = pos.x + 2;
		pos.z = 5;
		teapot.s_rigid_body_state.position = pos;
		Teapots.push_back(teapot);
	}


    // s_game_object.LoadRenderInfo(s_mesh_instance, s_effect_instance);
	
	
	
	// s_game_object1.LoadRenderInfo(s_mesh_instance, s_yellowSolid_Mat);
	s_movable_game_object = &Teapots[0];

	return Results::Success;
OnExit:

	return result;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	Graphics::cMesh::s_manager.Get(s_mesh_instance)->DecrementReferenceCount();


	for (auto mat : mat_List)
	{
		Graphics::cMaterial::s_manager.Get(mat)->DecrementReferenceCount();
	}

	

	 // auto instance1 = Graphics::cMesh::s_manager.Get(s_mesh_instance1);
	 // instance1 =nullptr;
	
	// auto instance2 = Graphics::cMesh::s_manager.Get(s_mesh_instance2);
	//  instance2= nullptr;


	for (auto mat : meshList)
	{
		auto instance = Graphics::cMesh::s_manager.Get(mat);
		instance = nullptr;
		Graphics::cMesh::s_manager.Release(mat);

	}

	for (auto mat : mat_List)
	{
		Graphics::cMaterial::s_manager.Release(mat);
	}

	return Results::Success;
}

void eae6320::cMyGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime,
	const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
	//Simulation Time
	//auto ctime= (float)cos(GetElapsedSecondCount_simulationTime());
	
	//Submission of color
	Graphics::Color c = Graphics::Color(0.4f, 0.4f, 0.4f, 1.0f);
	Graphics::SubmitClearColor(c);

	SubmitCameraInfo(s_camera_instance, i_elapsedSecondCount_sinceLastSimulationUpdate);

	s_direction_light.SubmitLightInfo(i_elapsedSecondCount_sinceLastSimulationUpdate);

	for (auto teapot : Teapots)
	{
		teapot.SubmitMeshWithEffect(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}
	
	

}

void eae6320::cMyGame::UpdateUserCtrl()
{
	{
		isHide = UserInput::IsKeyPressed(UserInput::KeyCodes::Shift);
		notAnimated = UserInput::IsKeyPressed(UserInput::KeyCodes::Control);

		//Camera Move Ctrl
		isLeft = UserInput::IsKeyPressed(UserInput::KeyCodes::Left);
		isRight = UserInput::IsKeyPressed(UserInput::KeyCodes::Right);
		isForward = UserInput::IsKeyPressed(UserInput::KeyCodes::Up);
		isBack = UserInput::IsKeyPressed(UserInput::KeyCodes::Down);
		isRotLeft = UserInput::IsKeyPressed('Q');
		isRotRight = UserInput::IsKeyPressed('E');	
		isRotDown = UserInput::IsKeyPressed('C');
		isRotUp = UserInput::IsKeyPressed('Z');
		
		
		//Object Move Ctrl
		isGoLeft = UserInput::IsKeyPressed('A');
		isGoRight = UserInput::IsKeyPressed('D');
		isGoUp = UserInput::IsKeyPressed('W');
		isGoDown = UserInput::IsKeyPressed('S');
		
		//Light Move Ctrl
		isLightRotUp = UserInput::IsKeyPressed('I');
		isLightRotDown = UserInput::IsKeyPressed('K');
		isLightRotLeft = UserInput::IsKeyPressed('J');
		isLightRotRight = UserInput::IsKeyPressed('L');
		




	
	}
}

void eae6320::cMyGame::UpdateGoMovement(const float i_elapsedSecondCount_sinceLastUpdate,Graphics::cGameObject *selectedGameobject)
{
	{
		Math::sVector movment_vector (0,0,0);
		
		if(isGoLeft)
		{
			movment_vector +=Math::sVector(-3, 0, 0);
		}
		else if(isGoRight)
		{
			movment_vector += Math::sVector(3, 0, 0);
		}

		if(isGoUp)
		{
			movment_vector +=Math::sVector(0, 3, 0);
		}
		else if(isGoDown)
		{
			movment_vector += Math::sVector(0, -3, 0);
		}

		
	
	 	selectedGameobject->s_rigid_body_state.velocity = movment_vector;
		selectedGameobject->s_rigid_body_state.Update(i_elapsedSecondCount_sinceLastUpdate);
	}
}

void eae6320::cMyGame::UpdateCameraMovement(const float i_elapsedSecondCount_sinceLastUpdate,
	Graphics::cGameObject* selectedGameobject)
{
	auto camera_matrix = s_camera_instance.s_transformation;
	Math::sVector movment_vector(0, 0, 0);
	float angularSpeed = 0;
	Math::sVector dirAxis;
	

	if (isLeft)
	{
		movment_vector += -camera_matrix.GetRightDirection()*2;
	}
	else if (isRight)
	{
		movment_vector += camera_matrix.GetRightDirection()*2;
	}
	else if (isForward)
	{
		movment_vector += -camera_matrix.GetBackDirection()*2;
	}
	else if (isBack)
	{
		movment_vector += camera_matrix.GetBackDirection() *2;
	}


  
	//FPS Rotate
   
	if(isRotLeft)
	{
		angularSpeed = 0.5f;
		dirAxis = Math::sVector(0, 1, 0);
	}else if(isRotRight)
	{
		angularSpeed = -0.5f;
		dirAxis = Math::sVector(0, 1, 0);
	}
	else if (isRotDown)
	{
		dirAxis = Math::sVector(1, 0, 0);
		angularSpeed = -0.5f;
	}else if (isRotUp)
	{
		dirAxis = Math::sVector(1,0,0);
		angularSpeed = 0.5f;
	}

	s_camera_instance.s_rigid_body_state.angularSpeed = angularSpeed;
	s_camera_instance.s_rigid_body_state.angularVelocity_axis_local = dirAxis;
	s_camera_instance.s_rigid_body_state.velocity = movment_vector;
	
	//After 
	s_camera_instance.UpdateRigidbody(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cMyGame::UpdateLightMovement(const float i_elapsedSecondCount_sinceLastUpdate)
{
	float angularSpeed = 0;
	Math::sVector dirAxis;

	if (isLightRotLeft)
	{
		angularSpeed = 0.5f;
		dirAxis = Math::sVector(0, 1, 0);
	}
	else if (isLightRotRight)
	{
		angularSpeed = -0.5f;
		dirAxis = Math::sVector(0, 1, 0);
	}
	else if (isLightRotDown)
	{
		dirAxis = Math::sVector(1, 0, 0);
		angularSpeed = -0.5f;
	}
	else if (isLightRotUp)
	{
		dirAxis = Math::sVector(1, 0, 0);
		angularSpeed = 0.5f;
	}

	s_direction_light.s_rigid_body_state.angularSpeed = angularSpeed;
	s_direction_light.s_rigid_body_state.angularVelocity_axis_local = dirAxis;
	s_direction_light.UpdateRigidbody(i_elapsedSecondCount_sinceLastUpdate);

}



