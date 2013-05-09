-- L_3T_DBUMP_GLOW.shader
-- Diffuse Bump Glow Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local bumpMap = Node("SampleNormalMap2D", "bumpMap")
	bumpMap.In.t = MTexture(1)
	bumpMap.In.tc = MTexCoord(1)

	local glowMap = Node("SampleTexture2D", "glowMap")
	glowMap.In.t = MTexture(2)
	glowMap.In.tc = MTexCoord(2)

	local invGlowMap = Node("OneMinus", "invGlowMap")
	invGlowMap.In.x = glowMap

	local diffuseLight = Node("Mul", "diffuseBlend")
	diffuseLight.In.x = diffuseTexture
	diffuseLight.In.y = invGlowMap

	local light = Node("LightDiffuse", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightTanVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.normal = bumpMap
	light.In.diffuseColor = diffuseLight

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light

	local glowBlend = Node("Mul", "glowBlend")
	glowBlend.In.x = glowMap
	glowBlend.In.y = diffuseTexture
		
	return { 
		Default = { color = glowBlend },
		Preview = { color = diffuseTexture }, 
		Diffuse1 = { color = mul } 
	}

end

Compile(Shader())