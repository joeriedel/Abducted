-- L_5T_BAKED_DSBUMP_GLOW_ADD_LUMSCALE.shader
-- Diffuse Specular Textured Bump Glow Shader (for prelit world surfaces)
-- Add two textures together, the second one is scaled by the luminosity of the first

Import("GenLight.import")
Import("LumScaleHelper.import")

local Shader = function()

	local completeMap = hGenLumScale(0, 2)
	local diffuseTexture = hGenLumScale(1, 2)
	
	return GenDSBakedBumpShader(completeMap, diffuseTexture, 2)

end

Compile(Shader())