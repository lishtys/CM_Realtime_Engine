/*
This class builds Mesh
*/
#ifndef EAE6320_CMESHBUILDER_H
#define EAE6320_CMESHBUILDER_H
// Includes
//=========

#include <Engine/Graphics/Configuration.h>
#include "Engine/Graphics/cMesh.h"

//
// // Class Declaration
// //==================
//
namespace eae6320
{
	namespace Assets
	{
		class cMeshBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			virtual cResult Build(const std::vector<std::string>& i_arguments) override;

			// Implementation
			//===============

		public:
			cResult LoadAsset(const char* const i_path, std::vector<eae6320::Graphics::VertexFormats::sMesh>& i_mesh,
				std::vector<uint16_t>& indexArray);
		};
	}
}
#endif	

