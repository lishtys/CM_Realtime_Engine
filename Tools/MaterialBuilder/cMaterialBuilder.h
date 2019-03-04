/*
This class builds Mesh
*/
#ifndef EAE6320_CMESHBUILDER_H
#define EAE6320_CMESHBUILDER_H
// Includes
//=========

#include <Engine/Graphics/Configuration.h>
#include "Engine/Graphics/cMesh.h"
#include "Engine/Graphics/cRenderState.h"
#include  <string>
#include "Engine/Graphics/Color.h"

//
// // Class Declaration
// //==================
//
namespace eae6320
{
	namespace Assets
	{
		class cMaterialBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			cResult LoadAsset(const char* m_path_source, std::string& i_path, Graphics::Color& i_color, std::string & texture_path);
		
			virtual cResult Build(const std::vector<std::string>& i_arguments) override;

			// Implementation
			//===============

		
		};
	}
}
#endif	

