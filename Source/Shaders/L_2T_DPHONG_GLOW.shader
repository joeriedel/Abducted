-- L_2T_DPHONG_GLOW.shader
-- Diffuse Phong Glow Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local glowMap = Node("SampleTexture2D", "glowMap")
	glowMap.In.t = MTexture(1)
	glowMap.In.tc = MTexCoord(1)

	local invGlowMap = Node("OneMinus", "invGlowMap")
	invGlowMap.In.x = glowMap

	local diffuseLight = Node("Mul", "diffuseBlend")
	diffuseLight.In.x = diffuseTexture
	diffuseLight.In.y = invGlowMap

	local light = Node("LightDiffuse", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.normal = MNormal(0)
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