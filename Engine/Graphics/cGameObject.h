#pragma once
#include "Engine/Physics/sRigidBodyState.h"
#include "cMesh.h"
#include "cEffect.h"
#include "cMaterial.h"

namespace eae6320
{
	namespace Graphics
	{
		class cGameObject
		{
		public:
			cGameObject();
			~cGameObject();
			void LoadRenderInfo(cMesh::Handle i_mesh_handle, cMaterial::Handle i_effect_handle);
			void SubmitMeshWithEffect(const float i_secondCountToExtrapolate);

			void ChangeMesh(cMesh* meshRef);
			void ChangeEffect(cEffect* effectRef);

			Physics::sRigidBodyState s_rigid_body_state;
		
			cMesh::Handle mesh_handle;
			cMaterial::Handle mat_handle;
			
			bool isVisable = true;
	
		};
	}

}


