#pragma once
#include "Engine/Physics/sRigidBodyState.h"
#include "Color.h"

class cDirectionLight
{
public:
	cDirectionLight();
	~cDirectionLight();


	void SubmitLightInfo(const float i_elapsedSecondCount_sinceLastUpdate);

	void UpdateRigidbody(const float i_elapsedSecondCount_sinceLastUpdate);

	eae6320::Physics::sRigidBodyState s_rigid_body_state;

	eae6320::Graphics::Color lightColor;
};

