#include "cMeshPrefab.h"


eae6320::Graphics::cMeshPrefab::cMeshPrefab()
{
}

void eae6320::Graphics::cMeshPrefab::LoadExternalData(std::vector<VertexFormats::sMesh> verticesArray,
	std::vector<uint16_t> indexArray)
{
	m_vertices_array = verticesArray;
	m_index_array = indexArray;
}

eae6320::Graphics::cMeshPrefab::~cMeshPrefab()
{
}

std::vector<eae6320::Graphics::VertexFormats::sMesh> eae6320::Graphics::cMeshPrefab::GetMeshData()
{
	return m_vertices_array;
}

std::vector<uint16_t> eae6320::Graphics::cMeshPrefab::GetIndexData()
{
	return m_index_array;
}
