-- L_4T_BAKED_DSBUMP_ADD.shader
-- Diffuse Specular Textured Bump Shader (for prelit world surfaces), with additive texture

Import("GenLight.import")

local Shader = function()

	local completeMap = Node("hSampleTexture2D", "completeMap")
	completeMap.In.t = MTexture(0)
	completeMap.In.tc = MTexCoord(0)

	local addMap = Node("hSampleTexture2D", "addMap")
	addMap.In.t = MTexture(3)
	addMap.In.tc = MTexCoord(3)

	local add = Node("hAdd", "add")
	add.In.x = completeMap
	add.In.y = addMap
		
	return GenDSBakedBumpShader(add)

end

Compile(Shader())