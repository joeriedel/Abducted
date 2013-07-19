-- L_3T_BAKED_DPHONG_ADD_LUMSCALE.shader
-- Diffuse Textured Phong Shader (for prelit world surfaces)
-- Add two textures together, the second one is scaled by the luminosity of the first

Import("GenLight.import")
Import("LumScaleHelper.import")

local Shader = function()

	local completeMap = hGenLumScale(0, 2)
	local diffuseTexture = hGenLumScale(1, 2)
	
	return GenDBakedPhongShader(completeMap, diffuseTexture)

end

Compile(Shader())
