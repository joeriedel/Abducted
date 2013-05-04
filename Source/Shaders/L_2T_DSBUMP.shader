-- L_2T_DSBUMP.shader
-- Diffuse Specular Bump Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local bumpMap = Node("SampleNormalMap2D", "bumpMap")
	bumpMap.In.t = MTexture(1)
	bumpMap.In.tc = MTexCoord(1)

	local light1ds = Node("Light1DS", "light1d")
	light1ds.In.lightVec = MLightDir(0)
	light1ds.In.lightHalf = MLightHalfDir(0)
	light1ds.In.lightDfColor = MLightDiffuseColor(0)
	light1ds.In.lightSpColor = MLightSpecularColor(0)
	light1ds.In.normal = bumpMap
	light1ds.In.diffuseColor = diffuseTexture
	light1ds.In.specularColor = MSpecularColor(0)

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light1ds
		
	return { Preview = { color = diffuseTexture }, Diffuse1 = { color = mul } }

end

Compile(Shader())