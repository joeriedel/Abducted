-- L_2T_DBUMP.shader
-- Diffuse Bump Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local bumpMap = Node("SampleNormalMap2D", "bumpMap")
	bumpMap.In.t = MTexture(1)
	bumpMap.In.tc = MTexCoord(1)

	local light1d = Node("Light1D", "light1d")
	light1d.In.lightVec = MLightDir(0)
	light1d.In.lightDfColor = MLightDiffuseColor(0)
	light1d.In.normal = bumpMap
	light1d.In.diffuseColor = diffuseTexture

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light1d
		
	return { Preview = { color = diffuseTexture }, Diffuse1 = { color = mul } }

end

Compile(Shader())