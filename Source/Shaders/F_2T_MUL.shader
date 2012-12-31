-- Multiplies two textures together

local Shader = function()

	local a = Node("SampleTexture2D", "Texture1")
	local b = Node("SampleTexture2D", "Texture2")
	local mul = Node("Mul", "Multiply Textures")
	local mulMat = Node("Mul", "Multiply By Material Color")
	
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	b.In.t = MTexture(1)
	b.In.tc = MTexCoord(1)
	
	mul.In.x = a
	mul.In.y = b
	
	mulMat.In.x = mul
	mulMat.In.y = MColor(0)
	
	-- Output final value
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
