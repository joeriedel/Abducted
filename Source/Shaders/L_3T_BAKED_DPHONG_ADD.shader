-- L_3T_BAKED_DPHONG.shader
-- Diffuse Textured Phong Shader (for prelit world surfaces), with additive texture

Import("GenLight.import")

local Shader = function()

	local completeMap = Node("hSampleTexture2D", "completeMap")
	completeMap.In.t = MTexture(0)
	completeMap.In.tc = MTexCoord(0)

	local addMap = Node("hSampleTexture2D", "addMap")
	addMap.In.t = MTexture(2)
	addMap.In.tc = MTexCoord(2)

	local add = Node("hAdd", "add")
	add.In.x = completeMap
	add.In.y = addMap
		
	return GenDBakedPhongShader(add)

end

Compile(Shader())

