// // Includes
// //=========
//
//
#include <Tools/MaterialBuilder/cMaterialBuilder.h>
#include <Engine/Platform/Platform.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include "Engine/Graphics/cRenderState.h"
#include <External/Lua/Includes.h>
#include <iostream>
#include <fstream>
#include "string.h"
#include "Engine/Graphics/Color.h"

// Inherited Implementation
//=========================

// Build
//------


namespace
{
	eae6320::cResult LoadTableValues(lua_State& io_luaState, std::string & i_path, eae6320::Graphics::Color& i_color, std::string & texture_path);
	eae6320::cResult LoadTableValuesColor(lua_State& io_luaState, eae6320::Graphics::Color& i_color);
	eae6320::cResult LoadTableValuesColorValue(lua_State& io_luaState, eae6320::Graphics::Color& i_color);
	eae6320::cResult LoadTableValuesPaths(lua_State& io_luaState, std::string & i_path, std::string & texture_path);
	eae6320::cResult LoadTableValuesPathsValue(lua_State& io_luaState, std::string & i_path);

}

namespace
{
	eae6320::cResult LoadTableValues(lua_State& io_luaState, std::string & i_path, eae6320::Graphics::Color& i_color, std::string & texture_path)
	{
		auto result = eae6320::Results::Success;
		if (!(result = LoadTableValuesColor(io_luaState, i_color)))
		{
			return result;
		}

		if (!(result = LoadTableValuesPaths(io_luaState, i_path, texture_path)))
		{
			return result;
		}

		return  result;
	}

	eae6320::cResult LoadTableValuesColor(lua_State& io_luaState, eae6320::Graphics::Color& i_color)
	{
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "tintColor";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValuesColorValue(io_luaState, i_color)))
			{
				goto OnExit;
			}
		}
		else
		{
			i_color = { 1,1,1,1 };
		}

	OnExit:

		// Pop the textures table
		lua_pop(&io_luaState, 1);

		return result;

	}

	eae6320::cResult LoadTableValuesColorValue(lua_State& io_luaState, eae6320::Graphics::Color& i_color)
	{
		auto result = eae6320::Results::Success;

		std::cout << "Iterating through every Color Value:" << std::endl;
		const auto valueCount = luaL_len(&io_luaState, -1);
		std::cout << " ValueCnt \t" << valueCount << std::endl;

		eae6320::Graphics::Color tintColor;
		if (lua_istable(&io_luaState, -1))
		{

			for (int i = 1; i <= valueCount; ++i)
			{
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				auto statusValue = lua_tonumber(&io_luaState, -1);
				tintColor.m_clear_color_[i - 1] = (float)statusValue;
				lua_pop(&io_luaState, 1);
			}
		}

		i_color = tintColor;
		return result;
	}

	eae6320::cResult LoadTableValuesPaths(lua_State& io_luaState, std::string & i_path, std::string & texture_path)
	{
		auto result = eae6320::Results::Success;
		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "effectPath";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValuesPathsValue(io_luaState, i_path)))
			{

			}
		}

		else
		{
			result = eae6320::Results::InvalidFile;
			std::cerr << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
			// Pop the textures table
		}

		lua_pop(&io_luaState, 1);



		constexpr auto* const diffuseKey = "albedo";
		lua_pushstring(&io_luaState, diffuseKey);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValuesPathsValue(io_luaState, texture_path)))
			{
				// Pop the textures table
			}
		}

		else
		{
			result = eae6320::Results::InvalidFile;
			std::cerr << "The value at \"" << diffuseKey << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
			// Pop the textures table

		}





		lua_pop(&io_luaState, 1);
		return result;
	}

	eae6320::cResult LoadTableValuesPathsValue(lua_State& io_luaState, std::string & i_path)
	{
		auto result = eae6320::Results::Success;

		std::cout << "Iterating through every RenderStates Value:" << std::endl;
		const auto valueCount = luaL_len(&io_luaState, -1);
		if (lua_istable(&io_luaState, -1))
		{

			for (int i = 1; i <= valueCount; ++i)
			{
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				auto pathVal = lua_tostring(&io_luaState, -1);
				std::cout << " path : \t" << pathVal << std::endl;
				const char* a = "data/";
				auto b = (std::string)a + pathVal;
				i_path = b;
				lua_pop(&io_luaState, 1);
			}
		}

		std::cout << "Done Paths!" << std::endl;


		return result;
	}

}


eae6320::cResult eae6320::Assets::cMaterialBuilder::LoadAsset(const char* m_path_source, std::string& i_arguments, Graphics::Color& tintColor, std::string & texture_path)
{
	auto result = Results::Success;

	// Create a new Lua state
	lua_State* luaState = nullptr;
	{
		luaState = luaL_newstate();
		if (!luaState)
		{
			result = eae6320::Results::OutOfMemory;


			if (luaState)
			{
				// If I haven't made any mistakes
				// there shouldn't be anything on the stack,
				// regardless of any errors encountered while loading the file:
			//	EAE6320_ASSERT(lua_gettop(luaState) == 0);

				lua_close(luaState);
				luaState = nullptr;
			}
		}
	}

	// Load the asset file as a "chunk",
	// meaning there will be a callable function at the top of the stack
	const auto stackTopBeforeLoad = lua_gettop(luaState);
	{
		const auto luaResult = luaL_loadfile(luaState, m_path_source);
		if (luaResult != LUA_OK)
		{
			result = eae6320::Results::Failure;

			// Pop the error message
			lua_pop(luaState, 1);
			if (luaState)
			{
				// If I haven't made any mistakes
				// there shouldn't be anything on the stack,
				// regardless of any errors encountered while loading the file:
			//	EAE6320_ASSERT(lua_gettop(luaState) == 0);

				lua_close(luaState);
				luaState = nullptr;
				OutputErrorMessageWithFileInfo(m_path_source,
					"Mesh data is not correct!");
				return Results::Failure;
			}
		}
	}
	// Execute the "chunk", which should load the asset
	// into a table at the top of the stack
	{
		constexpr int argumentCount = 0;
		constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
		constexpr int noMessageHandler = 0;
		const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
		if (luaResult == LUA_OK)
		{
			// A well-behaved asset file will only return a single value
			const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
			if (returnedValueCount == 1)
			{
				// A correct asset file _must_ return a table
				if (!lua_istable(luaState, -1))
				{
					result = eae6320::Results::InvalidFile;
					std::cerr << "Asset files must return a table (instead of a "
						<< luaL_typename(luaState, -1) << ")" << std::endl;
					// Pop the returned non-table value
					lua_pop(luaState, 1);
					if (luaState)
					{
						// If I haven't made any mistakes
						// there shouldn't be anything on the stack,
						// regardless of any errors encountered while loading the file:
				///		EAE6320_ASSERT(lua_gettop(luaState) == 0);

						lua_close(luaState);
						luaState = nullptr;
					}
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				std::cerr << "Asset files must return a single table (instead of "
					<< returnedValueCount << " values)" << std::endl;
				// Pop every value that was returned
				lua_pop(luaState, returnedValueCount);
				if (luaState)
				{
					// If I haven't made any mistakes
					// there shouldn't be anything on the stack,
					// regardless of any errors encountered while loading the file:
				//	EAE6320_ASSERT(lua_gettop(luaState) == 0);

					lua_close(luaState);
					luaState = nullptr;
				}
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			std::cerr << lua_tostring(luaState, -1);
			// Pop the error message
			lua_pop(luaState, 1);
			if (luaState)
			{
				// If I haven't made any mistakes
				// there shouldn't be anything on the stack,
				// regardless of any errors encountered while loading the file:
			//	EAE6320_ASSERT(lua_gettop(luaState) == 0);

				lua_close(luaState);
				luaState = nullptr;
			}
		}
	}
	// If this code is reached the asset file was loaded successfully,
	// and its table is now at index -1
	result = LoadTableValues(*luaState, i_arguments, tintColor, texture_path);

	// Pop the table
	lua_pop(luaState, 1);



	return  result;
}

eae6320::cResult eae6320::Assets::cMaterialBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = Results::Success;
	std::string  i_path;
	std::string  i_texture_path;

	Graphics::Color tintColor;;
	result = LoadAsset(m_path_source, i_path, tintColor, i_texture_path);

	if (!result)
	{
		OutputErrorMessageWithFileInfo(m_path_source,
			"Material data is not correct");
		return Results::Failure;


	}
	std::ofstream fout(m_path_target, std::ios::binary);


	const char* str = i_path.data();
	auto v_shader_len = static_cast<uint16_t>(strlen(str) + 1);
	fout.write((char*)&tintColor, sizeof(Graphics::Color));
	fout.write((char*)&v_shader_len, sizeof(uint16_t));
	fout.write(str, strlen(str) + 1);


	const char* str1 = i_texture_path.data();
	fout.write(str1, strlen(str1) + 1);
	std::cout << "Build Done" << i_path << std::endl;
	fout.close();

	return result;

}





