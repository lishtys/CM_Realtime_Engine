#pragma once
#include "Engine/Physics/sRigidBodyState.h"
#include "Engine/Math/cMatrix_transformation.h"


namespace eae6320
{
	namespace Graphics
	{
		class cCamera
		{
		public:
			cCamera();
			~cCamera();

			void SubmitCameraInfo(const float i_elapsedSecondCount_sinceLastUpdate);

			void UpdateRigidbody(const float i_elapsedSecondCount_sinceLastUpdate);

			Physics::sRigidBodyState s_rigid_body_state;  
			Math::cMatrix_transformation s_transformation;

			float i_verticalFieldOfView_inRadians = 1.0472f;
			float i_aspectRatio = 1;
			float i_z_nearPlane = .3f;
			float i_z_farPlane = 1000;
		private:
			void UpdateMatrix();
		};

	}
}


