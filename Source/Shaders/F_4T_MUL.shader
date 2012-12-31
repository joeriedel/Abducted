-- F_4T_MUL.sh
-- Multiplies two textures together

local Shader = function()

	local a = Node("SampleTexture2D", "Texture1")
	local b = Node("SampleTexture2D", "Texture2")
	local c = Node("SampleTexture2D", "Texture3")
	local d = Node("SampleTexture2D", "Texture4")
	local mul = Node("Mul4", "Multiply Textures")
	local mulMat = Node("Mul", "Multiply By Material Color")
	
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	b.In.t = MTexture(1)
	b.In.tc = MTexCoord(1)
	c.In.t = MTexture(2)
	c.In.tc = MTexCoord(2)
	d.In.t = MTexture(3)
	d.In.tc = MTexCoord(3)
	
	mul.In.x = a
	mul.In.y = b
	mul.In.z = c
	mul.In.w = d
	
	mulMat.In.x = mul
	mulMat.In.y = MColor(0)
	
	-- Output final value
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
