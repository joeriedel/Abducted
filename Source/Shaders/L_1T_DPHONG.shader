-- L_1T_DPHONG.shader
-- Diffuse Phong Shader

local Shader = function()

	local diffuseTexture = Node("SampleTexture2D", "diffuseTexture")
	diffuseTexture.In.t = MTexture(0)
	diffuseTexture.In.tc = MTexCoord(0)

	local light1d = Node("Light1D", "light1d")
	light1d.In.lightVec = MLightPos(0)
	light1d.In.lightDfColor = MLightDiffuseColor(0)
	light1d.In.normal = MNormal(0)
	light1d.In.diffuseColor = diffuseTexture

	local mul = Node("Mul", "mul")
	mul.In.x = MColor(0)
	mul.In.y = light1d

	local black = Node("VecZero", "black")
		
	return { 
		Default = { color = black },
		Preview = { color = diffuseTexture }, 
		Diffuse1 = { color = mul } 
	}

end

Compile(Shader())