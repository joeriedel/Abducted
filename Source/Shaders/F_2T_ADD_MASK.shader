-- F_2T_ADD_MASK.sh
-- Add two textures together
-- Emits alpha channel of T1

local Shader = function()

	local t1 = Node("SampleTexture2D", "T1")
	t1.In.t = MTexture(0)
	t1.In.tc = MTexCoord(0)
		
	local t2 = Node("SampleTexture2D", "T2")
	t2.In.t = MTexture(1)
	t2.In.tc = MTexCoord(1)
	
	local addTex = Node("Add", "Add Textures")
	addTex.In.x = t1
	addTex.In.y = t2
	
	local maskAlpha = Node("Swp_xyzw_XYZw_xyzW", "MaskAlpha")
	maskAlpha.In.XYZw = addTex
	maskAlpha.In.xyzW = t1
	
	local mulMat = Node("Mul", "Multiply By Material Color")
	mulMat.In.x = maskAlpha
	mulMat.In.y = MColor(0)
	
	return { Default = { color = mulMat } }
	
end

Compile(Shader())
