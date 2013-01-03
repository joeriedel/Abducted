-- F_3T_ADD.sh
-- Add three textures together

local Shader = function()

	local a = Node("SampleTexture2D", "Texture1")
	local b = Node("SampleTexture2D", "Texture2")
	local c = Node("SampleTexture2D", "Texture3")
	local add = Node("Add3", "Add Textures")
	local mulMat = Node("Mul", "Multiply By Material Color")
	
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	b.In.t = MTexture(1)
	b.In.tc = MTexCoord(1)
	c.In.t = MTexture(2)
	c.In.tc = MTexCoord(2)
	
	add.In.x = a
	add.In.y = b
	add.In.z = c
	
	mulMat.In.x = add
	mulMat.In.y = MColor(0)
	
	-- Output final value
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
