#include "cDirectionLight.h"
#include "cGameObject.h"


cDirectionLight::cDirectionLight()
{
	// lightColor = { 1,244.0f/256,214.0f/256,1 };
}


cDirectionLight::~cDirectionLight()
{
}

void cDirectionLight::SubmitLightInfo(const float i_elapsedSecondCount_sinceLastUpdate)
{
	auto	predictedPosition = s_rigid_body_state.PredictFuturePosition(i_elapsedSecondCount_sinceLastUpdate);
	auto	predictedQuaternion = s_rigid_body_state.PredictFutureOrientation(i_elapsedSecondCount_sinceLastUpdate);
	eae6320::Graphics::SubmitLightInfo(predictedQuaternion.CalculateForwardDirection().GetNormalized(),lightColor);
}

void cDirectionLight::UpdateRigidbody(const float i_elapsedSecondCount_sinceLastUpdate)
{
	s_rigid_body_state.Update(i_elapsedSecondCount_sinceLastUpdate);

}
