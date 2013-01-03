-- F_5TA_DETAIL.sh
-- Blend 5 textures together by alpha
-- Used for detail texturing. First texture channels alpha controls detail
-- blending of other two textures.
-- pixel = t1 * t2 * lerp(t3, t4, t1.a)
-- NOTE: t2, t3, t4 are sampled from second UV channel

local Shader = function()

	local t1 = Node("SampleTexture2D", "t1")
	local t2 = Node("SampleTexture2D", "t2")
	local t3 = Node("SampleTexture2D", "t3")
	local t4 = Node("SampleTexture2D", "t4")
	local t5 = Node("SampleTexture2D", "t5")
		
	t1.In.t = MTexture(0)
	t1.In.tc = MTexCoord(0)
	t2.In.t = MTexture(1)
	t2.In.tc = MTexCoord(1)
	t3.In.t = MTexture(2)
	t3.In.tc = MTexCoord(2)
	t4.In.t = MTexture(3)
	t4.In.tc = MTexCoord(3)
	t5.In.t = MTexture(4)
	t5.In.tc = MTexCoord(4)
	
	local lerp = Node("Lerp", "lerp")
	
	lerp.In.a = t4
	lerp.In.b = t5
	lerp.In.t = t2
	
	local mul = Node("Mul", "mul")
	mul.In.x = t1
	mul.In.y = t3
	local mul2 = Node("Mul", "mul2")
	mul2.In.x = mul
	mul2.In.y = lerp
		
	return { Default = { color = mul2 } }
	
end

Compile(Shader())
