-- L_2T_DSBUMP.shader
-- Diffuse Specular Bump Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local bumpMap = Node("SampleNormalMap2D", "bumpMap")
	bumpMap.In.t = MTexture(1)
	bumpMap.In.tc = MTexCoord(1)

	local light = Node("LightDiffuseSpecular", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightTanVec(0)
	light.In.lightHalf = MLightTanHalfVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.lightSpColor = MLightSpecularColor(0)
	light.In.normal = bumpMap
	light.In.diffuseColor = diffuseTexture
	light.In.specularColor = MSpecularColor(0)

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light
		
	local black = Node("VecZero", "black")

	return { 
		Default = { color = black },
		Preview = { color = diffuseTexture }, 
		DiffuseSpecular1 = { color = mul } 
	}

end

Compile(Shader())