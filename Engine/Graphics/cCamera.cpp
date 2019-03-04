#include "cCamera.h"
#include "cGameObject.h"


eae6320::Graphics::cCamera::cCamera()
{
	s_rigid_body_state.position = Math::sVector(0, 0, 8);
	s_rigid_body_state.velocity = Math::sVector(0, 0, 0);
	s_rigid_body_state.acceleration = Math::sVector(0, 0, 0);
	s_rigid_body_state.orientation = Math::cQuaternion();
	UpdateMatrix();
}


eae6320::Graphics::cCamera::~cCamera()
{
}

void eae6320::Graphics::cCamera::SubmitCameraInfo(const float i_elapsedSecondCount_sinceLastUpdate)
{
		  auto	predictedPosition = s_rigid_body_state.PredictFuturePosition(i_elapsedSecondCount_sinceLastUpdate);
		  auto	predictedQuaternion =s_rigid_body_state.PredictFutureOrientation(i_elapsedSecondCount_sinceLastUpdate);
          auto w2c=    Math::cMatrix_transformation::CreateWorldToCameraTransform(predictedQuaternion, predictedPosition);
          auto c2p= Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective
		  (i_verticalFieldOfView_inRadians, i_aspectRatio, i_z_nearPlane, i_z_farPlane);
		  Graphics::SubmitCameraInfo(w2c,c2p, predictedPosition);
}

void eae6320::Graphics::cCamera::UpdateRigidbody(const float i_elapsedSecondCount_sinceLastUpdate)
{
	s_rigid_body_state.Update(i_elapsedSecondCount_sinceLastUpdate);
	UpdateMatrix();
}

void eae6320::Graphics::cCamera::UpdateMatrix()
{
	s_transformation = Math::cMatrix_transformation(s_rigid_body_state.orientation, s_rigid_body_state.position);
}
