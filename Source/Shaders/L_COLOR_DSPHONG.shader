-- L_COLOR_DSPHONG.shader
-- Diffuse Specular Phong Shader

local Shader = function()

	local light1ds = Node("Light1DS", "light1d")
	light1ds.In.lightVec = MLightPos(0)
	light1ds.In.lightHalf = MLightHalfPos(0)
	light1ds.In.lightDfColor = MLightDiffuseColor(0)
	light1ds.In.lightSpColor = MLightSpecularColor(0)
	light1ds.In.normal = MNormal(0)
	light1ds.In.diffuseColor = MColor(0)
	light1ds.In.specularColor = MSpecularColor(0)
		
	return { Preview = { color = MColor(0) }, Diffuse1 = { color = light1ds } }

end

Compile(Shader())