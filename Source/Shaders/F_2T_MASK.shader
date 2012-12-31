-- F_2T_MASK.sh
-- Samples RGB from Texture 1 and A from Texture 2
-- Use the material blend mode Alpha for this shader.

local Shader = function()

	local t1 = Node("SampleTexture2D", "T1")
	t1.In.t = MTexture(0)
	t1.In.tc = MTexCoord(0)
		
	local t2 = Node("SampleTexture2D", "T2")
	t2.In.t = MTexture(1)
	t2.In.tc = MTexCoord(1)
	
	local maskAlpha = Node("Swp_xyzw_XYZw_xyzW", "MaskAlpha")
	maskAlpha.In.XYZw = t1
	maskAlpha.In.xyzW = t2
	
	local mulMat = Node("Mul", "Multiply By Material Color")
	mulMat.In.x = maskAlpha
	mulMat.In.y = MColor(0)
	
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
