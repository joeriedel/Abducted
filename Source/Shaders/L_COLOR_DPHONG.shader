-- L_COLOR_DPHONG.shader
-- Diffuse Phong Shader

local Shader = function()

	local light1d = Node("Light1D", "light1d")
	light1d.In.lightDir = MLightDir(0)
	light1d.In.lightDfColor = MLightDiffuseColor(0)
	light1d.In.normal = MNormal(0)
	light1d.In.diffuseColor = MColor(0)
		
	return { Preview = { color = MColor(0) }, Diffuse1 = { color = light1d } }

end

Compile(Shader())