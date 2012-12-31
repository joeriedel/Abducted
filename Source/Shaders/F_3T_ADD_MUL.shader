-- F_3T_ADD_MUL.sh
-- Add three textures together

local Shader = function()

	local a = Node("SampleTexture2D", "Texture1")
	local b = Node("SampleTexture2D", "Texture2")
	local c = Node("SampleTexture2D", "Texture3")
	local add = Node("Add", "Add Textures")
	local mul = Node("Mul", "Mul Textures")
	local mulMat = Node("Mul", "Multiply By Material Color")
	
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	b.In.t = MTexture(1)
	b.In.tc = MTexCoord(0)
	c.In.t = MTexture(2)
	c.In.tc = MTexCoord(1)
	
	add.In.x = a
	add.In.y = b
	mul.In.x = add
	mul.In.y = c
	
	local maskAlpha = Node("Swp_xyzw_XYZw_xyzW", "MaskAlpha")
	maskAlpha.In.XYZw = mul
	maskAlpha.In.xyzW = a
	
	mulMat.In.x = maskAlpha
	mulMat.In.y = MColor(0)
	
	-- Output final value
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
