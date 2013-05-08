-- L_COLOR_DPHONG.shader
-- Diffuse Phong Shader

local Shader = function()
	
	local light = Node("LightDiffuse", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.normal = MNormal(0)
	light.In.diffuseColor = MColor(0)
	
	local black = Node("VecZero", "black")
		
	return { 
		Default = { color = black },
		Preview = { color = MColor(0) }, 
		Diffuse1 = { color = light } 
	}

end

Compile(Shader())