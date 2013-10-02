-- L_5T_BAKED_DSBUMP_SMASK.shader
-- Diffuse Specular Textured Bump SMask Shader (for prelit world surfaces)

Import("GenLight.import")

local Shader = function()

	local completeMap = Node("hSampleTexture2D", "completeMap")
	completeMap.In.t = MTexture(0)
	completeMap.In.tc = MTexCoord(0)

	local addMap = Node("hSampleTexture2D", "addMap")
	addMap.In.t = MTexture(4)
	addMap.In.tc = MTexCoord(4)

	local add = Node("hAdd", "add")
	add.In.x = completeMap
	add.In.y = addMap
		
	return GenDSBakedBumpSpecMaskShader(add)

end

Compile(Shader())