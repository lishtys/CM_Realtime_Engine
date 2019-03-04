// // Includes
// //=========
//
//
#include <Tools/EffectBuilder/cEffectBuilder.h>
#include <Engine/Platform/Platform.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include "Engine/Graphics/cRenderState.h"
#include <External/Lua/Includes.h>
#include <iostream>
#include <fstream>
#include "string.h"
// Inherited Implementation
//=========================

// Build
//------


namespace
{
	eae6320::cResult LoadTableValues(lua_State& io_luaState, std::vector<std::string>& i_paths, uint8_t& render_state);
	eae6320::cResult LoadTableValuesColor(lua_State& io_luaState, uint8_t& render_state);
	eae6320::cResult LoadTableValuesColorValue(lua_State& io_luaState, uint8_t& render_state);
	eae6320::cResult LoadTableValuesPaths(lua_State& io_luaState, std::vector<std::string>& i_paths);
	eae6320::cResult LoadTableValuesPathsValue(lua_State& io_luaState, std::vector<std::string>& i_paths);

}

namespace
{
	eae6320::cResult LoadTableValues(lua_State& io_luaState, std::vector<std::string>& i_paths, uint8_t &render_state)
	{
		auto result = eae6320::Results::Success;
		if (!(result = LoadTableValuesColor(io_luaState, render_state)))
		{
			return result;
		}
		
		if (!(result = LoadTableValuesPaths(io_luaState, i_paths)))
		{
			return result;
		}

		return  result;
	}

	eae6320::cResult LoadTableValuesColor(lua_State& io_luaState, uint8_t&render_state)
	{
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "renderState";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);
		
		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValuesColorValue(io_luaState, render_state)))
			{
				goto OnExit;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			std::cerr << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
			goto OnExit;
		}

	OnExit:

		// Pop the textures table
		lua_pop(&io_luaState, 1);

		return result;

	}

	eae6320::cResult LoadTableValuesColorValue(lua_State& io_luaState, uint8_t& render_state)
	{
		auto result = eae6320::Results::Success;

		std::cout << "Iterating through every RenderStates Value:" << std::endl;
		const auto valueCount = luaL_len(&io_luaState, -1);
		uint8_t lua_render_state;
		if (lua_istable(&io_luaState, -1))
		{
			

			for (int i = 1; i <= valueCount; ++i)
			{
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				auto statusValue = lua_tointeger(&io_luaState, -1) > 0;
				std::cout << " statusValue \t" << statusValue << std::endl;

				switch (i)
				{
				case 1:
					if(statusValue)
					eae6320::Graphics::RenderStates::EnableDepthBuffering(lua_render_state);
					break;
				case 2:
					if (statusValue)
					eae6320::Graphics::RenderStates::EnableAlphaTransparency(lua_render_state);
					break;
				case 3:
					if (!statusValue)
						eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(lua_render_state);
					break;
				}
				lua_pop(&io_luaState, 1);
			}
		}

		render_state = lua_render_state;

		std::cout << "Done!" << std::endl;


		return result;
	}

	eae6320::cResult LoadTableValuesPaths(lua_State& io_luaState, std::vector<std::string>& i_paths)
	{
		auto result = eae6320::Results::Success;
		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "vfShaders";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValuesPathsValue(io_luaState, i_paths)))
			{
				goto OnExit;
			}
		}

			else
			{
				result = eae6320::Results::InvalidFile;
				std::cerr << "The value at \"" << key << "\" must be a table "
					"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
				goto OnExit;
			}

		OnExit:

			// Pop the textures table
			lua_pop(&io_luaState, 1);

			return result;
		}

	eae6320::cResult LoadTableValuesPathsValue(lua_State& io_luaState, std::vector<std::string>& i_paths)
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
				i_paths.push_back(b);
				lua_pop(&io_luaState, 1);
			}
		}

		std::cout << "Done Paths!" << std::endl;


		return result;
	}

}


eae6320::cResult eae6320::Assets::cEffectBuilder::LoadAsset(const char* m_path_source, std::vector<std::string>& i_arguments, uint8_t& render_state)
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
	result = LoadTableValues(*luaState,i_arguments,render_state);

	// Pop the table
	lua_pop(luaState, 1);

	

	return  result;
}

eae6320::cResult eae6320::Assets::cEffectBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = Results::Success;
	std::vector<std::string> paths;
	
	uint8_t render_state;;
	result = LoadAsset(m_path_source, paths, render_state);

	if (!result)
	{
		OutputErrorMessageWithFileInfo(m_path_source,
			"Effect data is not correct");
		return Results::Failure;
		

	}
	std::ofstream fout(m_path_target, std::ios::binary);
	const char* str = paths[0].data();
	auto v_shader_len = static_cast<uint16_t>(strlen(str)+1);
	
	fout.write((char*)&render_state, sizeof(uint8_t));
	fout.write((char*)&v_shader_len, sizeof(uint16_t));
	fout.write(str, strlen(str)+1);
	const char* str1 = paths[1].data();
	fout.write(str1, strlen(str1)+1);
	std::cout << "Build Done" << render_state << std::endl;

	fout.close();

	return result;

}





