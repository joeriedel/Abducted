-- F_4TA_DETAIL.sh
-- Blend 4 textures together by alpha
-- Used for detail texturing. Second texture controls detail
-- blending of other two textures.
-- pixel = t1 * lerp(t3, t4, t2)

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
		
	lerp.In.a = t3
	lerp.In.b = t4
	lerp.In.t = t2
	
	local mul = Node("Mul", "mul")
	mul.In.x = t1
	mul.In.y = lerp
		
	return { Default = { color = mul } }
	
end

Compile(Shader())
