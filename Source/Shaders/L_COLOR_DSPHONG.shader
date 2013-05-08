-- L_COLOR_DSPHONG.shader
-- Diffuse Specular Phong Shader

local Shader = function()

	local light = Node("LightDiffuseSpecular", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightVec(0)
	light.In.lightHalf = MLightHalfVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.lightSpColor = MLightSpecularColor(0)
	light.In.normal = MNormal(0)
	light.In.diffuseColor = MColor(0)
	light.In.specularColor = MSpecularColor(0)
	
	local black = Node("VecZero", "black")
		
	return { 
		Default = { color = black },
		Preview = { color = MColor(0) }, 
		DiffuseSpecular1 = { color = light } 
	}

end

Compile(Shader())