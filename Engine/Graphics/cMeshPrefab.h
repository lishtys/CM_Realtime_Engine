#pragma once
#include "cMesh.h"

namespace eae6320
{
	namespace Graphics
	{
		class cMeshPrefab
		{
		public:
			cMeshPrefab();
			~cMeshPrefab();
			void LoadExternalData(std::vector<VertexFormats::sMesh> verticesArray, std::vector<uint16_t> indexArray);
			std::vector<VertexFormats::sMesh>	GetMeshData();
			std::vector<uint16_t> GetIndexData();
		
		private:
			std::vector<VertexFormats::sMesh> m_vertices_array;
			std::vector<uint16_t> m_index_array;
		};
	}
}


