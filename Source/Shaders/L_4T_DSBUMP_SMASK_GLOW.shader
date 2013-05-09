-- L_4T_DSBUMP_SMASK_GLOW.shader
-- Diffuse Specular Bump Shader
-- Specular comes from shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local bumpMap = Node("SampleNormalMap2D", "bumpMap")
	bumpMap.In.t = MTexture(1)
	bumpMap.In.tc = MTexCoord(1)
	
	local specularTexture = Node("SampleTexture2D", "diffuseTexture")
	specularTexture.In.t = MTexture(2)
	specularTexture.In.tc = MTexCoord(2)

	local glowMap = Node("SampleTexture2D", "glowMap")
	glowMap.In.t = MTexture(3)
	glowMap.In.tc = MTexCoord(3)

	local invGlowMap = Node("OneMinus", "invGlowMap")
	invGlowMap.In.x = glowMap

	local diffuseLight = Node("Mul", "diffuseBlend")
	diffuseLight.In.x = diffuseTexture
	diffuseLight.In.y = invGlowMap

	local light = Node("LightDiffuseSpecular", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightTanVec(0)
	light.In.lightHalf = MLightTanHalfVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.lightSpColor = MLightSpecularColor(0)
	light.In.normal = bumpMap
	light.In.diffuseColor = diffuseLight

	-- preserve specular exponent
	local toHALF4 = Node("toHALF4", "toHALF4")
	toHALF4.In.x = specularTexture

	local specWithExponent = Node("hSwp_xyzw_XYZw_xyzW", "specWithExponent")
	specWithExponent.In.XYZw = toHALF4
	specWithExponent.In.xyzW = MSpecularColor(0)

	light.In.specularColor = specWithExponent

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light

	local glowBlend = Node("Mul", "glowBlend")
	glowBlend.In.x = glowMap
	glowBlend.In.y = diffuseTexture

	return { 
		Default = { color = glowBlend },
		Preview = { color = diffuseTexture }, 
		DiffuseSpecular1 = { color = mul } 
	}

end

Compile(Shader())