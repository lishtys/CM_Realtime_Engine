// // Includes
// //=========
//
//
#include <Tools/MeshBuilder/cMeshBuilder.h>
#include <Engine/Platform/Platform.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include <External/Lua/Includes.h>
#include "Engine/Graphics/cMesh.h"
#include <iostream>
#include <fstream>
#include "Engine/Time/Time.h"
// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cMeshBuilder::Build(const std::vector<std::string>& i_arguments)
{
	Time::Initialize();
	auto result = Results::Success;


	std::vector<Graphics::VertexFormats::sMesh> veticesArray;
	std::vector<uint16_t> indexArray;
	auto  begin = Time::GetCurrentSystemTimeTickCount();

	result=	LoadAsset(m_path_source, veticesArray, indexArray);
	
	if(!result)
	{
		OutputErrorMessageWithFileInfo(m_path_source,
			"Mesh data is not correct");
		return Results::Failure;

	}

	auto  end = Time::GetCurrentSystemTimeTickCount();

	auto res = end - begin;
	std::cout << "Time :" << Time::ConvertTicksToSeconds(res) << std::endl;
	std::ofstream fout(m_path_target, std::ios::binary);



	uint16_t verticesCnt = static_cast<uint16_t>(veticesArray.size());
	uint16_t indexArrayCnt = static_cast<uint16_t>(indexArray.size());
	fout.write((char*)&verticesCnt, sizeof(uint16_t));
	
	std::cout << "Vertices Cnt :" << verticesCnt << std::endl;
	fout.write((char*)&veticesArray[0], verticesCnt * sizeof(Graphics::VertexFormats::sMesh));
	fout.write((char*)&indexArrayCnt, sizeof(uint16_t));
	std::cout << "IndexArray Cnt :" << indexArrayCnt << std::endl;
	fout.write((char*)&indexArray[0], indexArrayCnt*sizeof(uint16_t));


	fout.close();



	return result;

}




namespace
{

	namespace
	{
		eae6320::cResult LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);
		eae6320::cResult LoadTableValues_Vertex(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);
		eae6320::cResult LoadTableValues_Vertex_Path(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);
		eae6320::cResult LoadTableValues_Index(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);
		eae6320::cResult LoadTableValues_Index_Path(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray);
	}

	namespace
	{
		eae6320::cResult LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray)
		{
			auto result = eae6320::Results::Success;

			if (!(result = LoadTableValues_Vertex(io_luaState, i_mesh, indexArray)))
			{
				return result;
			}
			if (!(result = LoadTableValues_Index(io_luaState, i_mesh, indexArray)))
			{
				return result;
			}

			return result;
		}

		eae6320::cResult LoadTableValues_Vertex(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray)
		{
			auto result = eae6320::Results::Success;

			// Right now the asset table is at -1.
			// After the following table operation it will be at -2
			// and the "textures" table will be at -1:
			constexpr auto* const key = "VerArray";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			// It can be hard to remember where the stack is at
			// and how many values to pop.
			// One strategy I would suggest is to always call a new function
			// When you are at a new level:
			// Right now we know that we have an original table at -2,
			// and a new one at -1,
			// and so we _know_ that we always have to pop at least _one_
			// value before leaving this function
			// (to make the original table be back to index -1).
			// If we don't do any further stack manipulation in this function
			// then it becomes easy to remember how many values to pop
			// because it will always be one.
			// This is the strategy I'll take in this example
			// (look at the "OnExit" label):
			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_Vertex_Path(io_luaState, i_mesh, indexArray)))
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

		eae6320::cResult LoadTableValues_Index_Path(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray)
		{
			// Right now the asset table is at -2
			// and the textures table is at -1.
			// NOTE, however, that it doesn't matter to me in this function
			// that the asset table is at -2.
			// Because I've carefully called a new function for every "stack level"
			// The only thing I care about is that the textures table that I care about
			// is at the top of the stack.
			// As long as I make sure that when I leave this function it is _still_
			// at -1 then it doesn't matter to me at all what is on the stack below it.

			auto result = eae6320::Results::Success;

			std::cout << "Iterating through every Index path:" << std::endl;
			const auto textureCount = luaL_len(&io_luaState, -1);
			std::cout << "Index Lua Count\t" << textureCount << std::endl;
			if (lua_istable(&io_luaState, -1))
			{
				for (int i = 1; i <= textureCount; ++i)
				{

					lua_pushinteger(&io_luaState, i);
					lua_gettable(&io_luaState, -2);
					const auto vNum = luaL_len(&io_luaState, -1);
					for (int j = 1; j <= vNum; ++j)
					{

						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						// std::cout << "\t" << lua_tostring(&io_luaState, -1) << std::endl;
						indexArray.push_back(static_cast<uint16_t>(lua_tonumber(&io_luaState, -1)));
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);
				}
			}

			return result;
		}
		eae6320::cResult LoadTableValues_Index(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray)
		{
			auto result = eae6320::Results::Success;

			// Right now the asset table is at -1.
			// After the following table operation it will be at -2
			// and the "textures" table will be at -1:
			constexpr auto* const key = "IndexArray";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			// It can be hard to remember where the stack is at
			// and how many values to pop.
			// One strategy I would suggest is to always call a new function
			// When you are at a new level:
			// Right now we know that we have an original table at -2,
			// and a new one at -1,
			// and so we _know_ that we always have to pop at least _one_
			// value before leaving this function
			// (to make the original table be back to index -1).
			// If we don't do any further stack manipulation in this function
			// then it becomes easy to remember how many values to pop
			// because it will always be one.
			// This is the strategy I'll take in this example
			// (look at the "OnExit" label):
			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_Index_Path(io_luaState, i_mesh, indexArray)))
				{
					goto OnExit;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
			
				goto OnExit;
			}


		OnExit:

			// Pop the textures table
			lua_pop(&io_luaState, 1);

			return result;
		}

		eae6320::cResult LoadTableValues_Vertex_Path(lua_State& io_luaState, std::vector<eae6320::Graphics::VertexFormats::sMesh >&i_mesh,
			std::vector<uint16_t >&indexArray)
		{
			// Right now the asset table is at -2
			// and the textures table is at -1.
			// NOTE, however, that it doesn't matter to me in this function
			// that the asset table is at -2.
			// Because I've carefully called a new function for every "stack level"
			// The only thing I care about is that the textures table that I care about
			// is at the top of the stack.
			// As long as I make sure that when I leave this function it is _still_
			// at -1 then it doesn't matter to me at all what is on the stack below it.

			auto result = eae6320::Results::Success;

			std::cout << "Iterating through every Vertex path:" << std::endl;
			const auto InfoCount = luaL_len(&io_luaState, -1);
			std::cout << " Vertex Count\t" << InfoCount << std::endl;
			if (lua_istable(&io_luaState, -1))
			{
				for (int i = 1; i <= InfoCount; ++i)
				{
					eae6320::Graphics::VertexFormats::sMesh s_mesh;
					lua_pushinteger(&io_luaState, i);
					lua_gettable(&io_luaState, -2);


					lua_pushstring(&io_luaState, "position");
					lua_gettable(&io_luaState, -2);


					const auto vNum = luaL_len(&io_luaState, -1);
					for (int j = 1; j <= vNum; ++j)
					{

						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						if (j == 1) s_mesh.x = static_cast<float>(lua_tonumber(&io_luaState, -1));
						if (j == 2) s_mesh.y = static_cast<float>(lua_tonumber(&io_luaState, -1));
						if (j == 3) s_mesh.z = static_cast<float>(lua_tonumber(&io_luaState, -1));
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);


					lua_pushstring(&io_luaState, "color");
					lua_gettable(&io_luaState, -2);
					const auto cNum = luaL_len(&io_luaState, -1);

					for (int j = 1; j <= cNum; ++j)
					{

						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						if (j == 1) s_mesh.r = static_cast<uint8_t> (static_cast<float>(lua_tonumber(&io_luaState, -1)) * 255);
						if (j == 2) s_mesh.g = static_cast<uint8_t> (static_cast<float>(lua_tonumber(&io_luaState, -1)) * 255);
						if (j == 3) s_mesh.b = static_cast<uint8_t> (static_cast<float>(lua_tonumber(&io_luaState, -1)) * 255);
						if (j == 3) s_mesh.a = static_cast<uint8_t> (static_cast<float>(lua_tonumber(&io_luaState, -1)) * 255);
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);


					lua_pushstring(&io_luaState, "uv");
					lua_gettable(&io_luaState, -2);
					const auto uvNum = luaL_len(&io_luaState, -1);

					for (int j = 1; j <= uvNum; ++j)
					{

						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						if (j == 1) s_mesh.u = static_cast<float> (static_cast<float>(lua_tonumber(&io_luaState, -1)) );
						if (j == 2) s_mesh.v = 1-static_cast<float> (static_cast<float>(lua_tonumber(&io_luaState, -1)) );
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);

					lua_pushstring(&io_luaState, "normal");
					lua_gettable(&io_luaState, -2);
					const auto nNum = luaL_len(&io_luaState, -1);

					for (int j = 1; j <= cNum; ++j)
					{

						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						if (j == 1) s_mesh.nx = static_cast<float> (static_cast<float>(lua_tonumber(&io_luaState, -1)));
						if (j == 2) s_mesh.ny = static_cast<float> (static_cast<float>(lua_tonumber(&io_luaState, -1)));
						if (j == 3) s_mesh.nz = static_cast<float> (static_cast<float>(lua_tonumber(&io_luaState, -1)));
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);


					lua_pop(&io_luaState, 1);
					i_mesh.push_back(s_mesh);
				}
			}

			return result;
		}
	}




}

eae6320::cResult eae6320::Assets::cMeshBuilder::LoadAsset(const char* const i_path,
	std::vector<eae6320::Graphics::VertexFormats::sMesh>& i_mesh, std::vector<uint16_t>& indexArray)
{
	auto result = eae6320::Results::Success;

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
		const auto luaResult = luaL_loadfile(luaState, i_path);
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
	result = LoadTableValues(*luaState, i_mesh, indexArray);

	// Pop the table
	lua_pop(luaState, 1);

	return result;
}