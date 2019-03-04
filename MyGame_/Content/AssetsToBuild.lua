--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	shaders =
	{
		{ path = "Shaders/Vertex/vertexInputLayout.shader", arguments = { "vertex" } },
	},
	meshes =
	{
--		{ path = "Meshes/teapot.mesh"},
--		{ path = "Meshes/Plane.mesh"},
--	    { path = "Meshes/Circle.mesh"},
		{ path = "Meshes/Cube.mesh"},
		{ path = "Meshes/Sphere.mesh"},
		{ path = "Meshes/Plane.mesh"},
	},
	materials =
	{
		{ path = "Materials/TexMatYo.material"},
		{ path = "Materials/TexMatRem.material"},
		{ path = "Materials/TexMatAynami.material"},
		{ path = "Materials/TexEarth.material"},
		{ path = "Materials/TexMatSuits.material"},
		{ path = "Materials/RiverAnim.material"},
	},
}
