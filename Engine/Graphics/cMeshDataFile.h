// // Includes
// //=========
//
// #include "Configuration.h"
//
// #include <Engine/Assets/ReferenceCountedAssets.h>
//
// #include <cstdint>
// #include <Engine/Assets/cHandle.h>
// #include <Engine/Assets/cManager.h>
// #include <Engine/Results/Results.h>
// #include "cMesh.h"
//
//
// // Forward Declarations
// //=====================
//
// namespace eae6320
// {
// 	namespace Platform
// 	{
// 		struct sDataFromFile;
// 	}
// }
//
// namespace eae6320
// {
// 	namespace Graphics
// 	{
// 		class  cMeshDataFile
// 		{
// 		public:
//
// 			// Assets
// 			//-------
//
// 			using Handle = Assets::cHandle<cMesh>;
// 			static Assets::cManager<cMesh> s_manager;
// 		
// 		
//
// 			static cResult Load(const char* const i_path, cMesh*& o_mesh_data);
//
// 			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cMeshDataFile);
//
// 			// Reference Counting
// 			//-------------------
// 			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();
// 			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
// 		private:
//
// 			// Initialization / Clean Up
// 			//--------------------------
//
// 			cResult Initialize(const char* const i_path, const Platform::sDataFromFile& i_loadedMesh);
// 			cResult CleanUp();
//
// 			cMeshDataFile();
// 			~cMeshDataFile();
// 		
// 		};
//
//
//
//
// 	}
// }