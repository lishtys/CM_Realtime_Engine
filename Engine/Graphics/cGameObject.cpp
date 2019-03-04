#include "cGameObject.h"
#include "Engine/UserOutput/UserOutput.h"


eae6320::Graphics::cGameObject::cGameObject()
{
	s_rigid_body_state.position = Math::sVector(0, 0, 0);
	s_rigid_body_state.velocity = Math::sVector(0, 0, 0);
	s_rigid_body_state.acceleration = Math::sVector(0, 0, 0);
	s_rigid_body_state.orientation = Math::cQuaternion();
}


eae6320::Graphics::cGameObject::~cGameObject()
{
}

void eae6320::Graphics::cGameObject::LoadRenderInfo(cMesh::Handle i_meshHandle,cMaterial::Handle i_mat_handle)
{
	mesh_handle = i_meshHandle;
	mat_handle = i_mat_handle;

}

void eae6320::Graphics::cGameObject::SubmitMeshWithEffect(const float i_secondCountToExtrapolate)
{
	if(isVisable)
	{
		auto m_mesh_instance = cMesh::s_manager.Get(mesh_handle);
		auto m_mat_instance = cMaterial::s_manager.Get(mat_handle);

		Graphics::SubmitMeshEffectPairs(mesh_handle.GetIndex(), mat_handle.GetIndex(), m_mat_instance->GetEffectIdx(),m_mesh_instance, m_mat_instance, s_rigid_body_state.PredictFutureOrientation(i_secondCountToExtrapolate),
			s_rigid_body_state.PredictFuturePosition(i_secondCountToExtrapolate));
	}
}

void eae6320::Graphics::cGameObject::ChangeMesh(cMesh* meshRef)
{
}

void eae6320::Graphics::cGameObject::ChangeEffect(cEffect* effectRef)
{
}





