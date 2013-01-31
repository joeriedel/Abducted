-- F_2T_SCALE.sh
-- Samples RGB from Texture 1 and A from Texture 2
-- Use the material blend mode Add for this shader.

local Shader = function()

	local t1 = Node("SampleTexture2D", "T1")
	t1.In.t = MTexture(0)
	t1.In.tc = MTexCoord(0)
			
	local t2 = Node("SampleTexture2D", "T2")
	t2.In.t = MTexture(1)
	t2.In.tc = MTexCoord(1)
	
	local sw_alpha = Node("Sw_wwww", "Sw_alpha")
	sw_alpha.In.x = t2
	
	local mul = Node("Mul", "Mul")
	mul.In.x = t1
	mul.In.y = sw_alpha
	
	local mulMat = Node("Mul", "Multiply By Material Color")
	mulMat.In.x = mul
	mulMat.In.y = MColor(0)
	
	return { Default = { color = mulMat } }
	
end

Compile(Shader())