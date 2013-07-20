-- L_5T_BAKED_DSBUMP_SMASK_ADD_LUMSCALE.shader
-- Diffuse Specular Textured Bump SMask Shader (for prelit world surfaces)
-- Add two textures together, the second one is scaled by the luminosity of the first

Import("GenLight.import")
Import("LumScaleHelper.import")

local Shader = function()

	local completeMap = hGenLumScale(0, 2)
	local diffuseTexture = hGenLumScale(1, 2)
	
	return GenDSBakedBumpSpecMaskShader(completeMap, diffuseTexture)

end

Compile(Shader())
