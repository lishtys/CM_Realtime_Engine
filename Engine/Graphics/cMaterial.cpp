#include "cMaterial.h"

eae6320::Assets::cManager<eae6320::Graphics::cMaterial> eae6320::Graphics::cMaterial::s_manager;
eae6320::Graphics::cMaterial::cMaterial()
{
}

eae6320::Graphics::cMaterial::~cMaterial()
{
	CleanUp();
}

eae6320::cResult eae6320::Graphics::cMaterial::CleanUp()
{
	auto result = eae6320::Results::Success;
	if (m_effect_handle)
	{
		const auto localResult = cEffect::s_manager.Release(m_effect_handle);
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}
	
	if (m_texture_handle)
	{
		const auto localResult = cTexture::s_manager.Release(m_texture_handle);
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}
	return result;
}

eae6320::cResult eae6320::Graphics::cMaterial::InitialData(const char filePath[], const char textureFilePath[], Color t_color)
{
	auto result = eae6320::Results::Success;
	if (!(result = Graphics::cEffect::EffectFactory(filePath,m_effect_handle)))
	{

	}

	result=	Graphics::cTexture::s_manager.Load(textureFilePath, m_texture_handle);
	
	
	m_color = t_color;

	return result;

}


eae6320::cResult eae6320::Graphics::cMaterial::Load(const std::string& i_path, cMaterial*& o_shader,
	const ShaderTypes::eType i_type)
{
	auto result = eae6320::Results::Success;

	
	auto instance = new Graphics::cMaterial;
	o_shader = instance;
	//Load Binary Asset
	Platform::sDataFromFile pData;
	Platform::LoadBinaryFile(i_path.c_str(), pData);
	uintptr_t start = reinterpret_cast<uintptr_t>(pData.data);
	uintptr_t current = start;
	
	Color t_color = *reinterpret_cast<Color*>(current);
	current += sizeof(Color);
	uint16_t  pathLen = *reinterpret_cast<uint16_t*>(current);
	current += sizeof(uint16_t);
	
	auto path = reinterpret_cast<char*>(current);
	current += sizeof(char)*(pathLen);

	auto path1 = reinterpret_cast<char*>(current);

	instance->InitialData(path, path1,t_color);
	return  result;
}

uint_fast32_t eae6320::Graphics::cMaterial::GetEffectIdx()
{
	return m_effect_handle.GetIndex();
}
