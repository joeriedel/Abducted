-- L_1T_DPHONG.shader
-- Diffuse Phong Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local light = Node("LightDiffuse", "light")
	light.In.lightPos = MLightPos(0)
	light.In.fragPos = MVertex(0)
	light.In.lightVec = MLightVec(0)
	light.In.lightDfColor = MLightDiffuseColor(0)
	light.In.normal = MNormal(0)
	light.In.diffuseColor = diffuseTexture

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light

	local black = Node("VecZero", "black")
		
	return { 
		Default = { color = black },
		Preview = { color = diffuseTexture }, 
		Diffuse1 = { color = mul } 
	}

end

Compile(Shader())