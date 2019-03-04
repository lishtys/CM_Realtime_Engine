#pragma once
#include "cMesh.h"
#include <External/Lua/Includes.h>
#include <iostream>
#include "Tools/MeshBuilder/cMeshBuilder.h"
#include "Engine/Time/Time.h"
#include "cMaterial.h"

eae6320::Assets::cManager<eae6320::Graphics::cMesh> eae6320::Graphics::cMesh::s_manager;



namespace
{
	namespace
	{

	}
}


eae6320::cResult eae6320::Graphics::cMesh::LoadAsset(const  std::string&  i_path, std::vector<eae6320::Graphics::VertexFormats::sMesh>& i_mesh,
	std::vector<uint16_t>& indexArray)
{
	auto result = eae6320::Results::Success;
//  Load Binary Asset

	Platform::sDataFromFile pData;
	auto  begin = Time::GetCurrentSystemTimeTickCount();
	Platform::LoadBinaryFile(i_path.c_str(), pData);

	uintptr_t start = reinterpret_cast<uintptr_t>(pData.data);
	uintptr_t current = start;
	uint16_t  vCnt = *reinterpret_cast<uint16_t*>(current);
	current += sizeof(vCnt);
	auto vData = reinterpret_cast<VertexFormats::sMesh*>(current);
	current += sizeof(VertexFormats::sMesh)*vCnt;
	uint16_t  iCnt = *reinterpret_cast<uint16_t*>(current);
	current += sizeof(iCnt);
	auto iData = reinterpret_cast<uint16_t*>(current);
	current+= sizeof(uint16_t)*iCnt;
	auto  end = Time::GetCurrentSystemTimeTickCount();

	auto res = end - begin;
	auto duration = Time::ConvertTicksToSeconds(res);
	std::cout << i_path << " --Time :" << duration << std::endl;

	if(current!=start+pData.size)
	{
		return  Results::Failure;
	}
	 std::vector<VertexFormats::sMesh> meshes(vData,vData+vCnt);
	 i_mesh = meshes;
	
	std::vector<uint16_t> indexs(iData,iData+iCnt);
	 indexArray = indexs;

	return result;
}

eae6320::cResult eae6320::Graphics::cMesh::Load(const  std::string&  i_path, eae6320::Graphics::cMesh*& o_mesh_data, const ShaderTypes::eType i_type)
{
	auto result = eae6320::Results::Success;
	std::vector<VertexFormats::sMesh> veticesArray;
	std::vector<uint16_t> indexArray;


	auto instance = new cMesh;
	o_mesh_data = instance;

	if(!instance->LoadAsset(i_path, veticesArray, indexArray))
	{
		return eae6320::Results::Failure;
	}
	else
	{
		result = instance->InitializeGeometry(veticesArray, indexArray);
	}



	return  result;
}