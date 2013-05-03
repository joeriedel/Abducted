-- L_1T_DSPHONG.shader
-- Diffuse Specular Phong Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local light1ds = Node("Light1DS", "light1d")
	light1ds.In.lightVec = MLightPos(0)
	light1ds.In.lightHalf = MLightHalfPos(0)
	light1ds.In.lightDfColor = MLightDiffuseColor(0)
	light1ds.In.lightSpColor = MLightSpecularColor(0)
	light1ds.In.normal = MNormal(0)
	light1ds.In.diffuseColor = diffuseTexture
		
	return { Preview = { color = diffuseTexture }, Diffuse1 = { color = light1ds } }

end

Compile(Shader())