-- F_4T_DETAIL.sh
-- Blend 4 textures together by alpha
-- Used for detail texturing. First texture channels alpha controls detail
-- blending of other two textures.
-- pixel = t1 * t2 * lerp(t3, t4, t1.a)
-- NOTE: t2, t3, t4 are sampled from second UV channel

local Shader = function()

	local t1 = Node("SampleTexture2D", "t1")
	local t2 = Node("SampleTexture2D", "t2")
	local t3 = Node("SampleTexture2D", "t3")
	local t4 = Node("SampleTexture2D", "t4")
		
	t1.In.t = MTexture(0)
	t1.In.tc = MTexCoord(0)
	t2.In.t = MTexture(1)
	t2.In.tc = MTexCoord(1)
	t3.In.t = MTexture(2)
	t3.In.tc = MTexCoord(2)
	t4.In.t = MTexture(3)
	t4.In.tc = MTexCoord(3)
	
	local lerp = Node("Lerp", "lerp")
	local t1a = Node("Sw_wwww", "t1a")
	
	t1a.In.x = t1
	lerp.In.a = t3
	lerp.In.b = t4
	lerp.In.t = t1a
	
	local mul = Node("Mul", "mul")
	mul.In.x = t1
	mul.In.y = t2
	local mul2 = Node("Mul", "mul2")
	mul2.In.x = mul
	mul2.In.y = lerp
		
	return { Default = { color = mul2 } }
	
end

Compile(Shader())
