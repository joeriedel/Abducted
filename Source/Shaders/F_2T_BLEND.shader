-- F_2T_BLEND.sh
-- Blend 2 textures together by alpha
-- Should be identical to 2 alpha blended textures done in 2 passes
-- Use the material blend mode AddBlend for this shader.

local Shader = function()

	local a = Node("SampleTexture2D", "Texture1")
	local aa = Node("Sw_wwww", "SwTexture1AAAA")
	local iaa = Node("OneMinus", "InvTexture1Alpha")
	local amul_a = Node("Mul", "Texture2MulAlpha")
	local b = Node("SampleTexture2D", "Texture2")
	local ba = Node("Sw_wwww", "SwTexture2AAAA")
	local iba = Node("OneMinus", "InvTexture2Alpha")
	local sw_rgb = Node("Sw_xyz", "MaskRGB")
	local sw_alpha = Node("Sw_w", "MaskAlpha")
	local mulAlpha = Node("Mul", "MulAlpha")
	local mul3 = Node("Mul3", "Mul3")
	local add = Node("Add", "Add")
	local mulMat = Node("Mul", "Multiply By Material Color")
	local invAlpha = Node("OneMinus", "InvAlpha")
	local mulMatAlpha = Node("Mul", "MulMatAlpha")

	-- (1 - (1-xa) * (1-ya)) + (y*ya*(1-xa)) + (x*xa)
	-- alpha output = (f0 * (1-xa) * (1-ya))
	-- color output = (y*ya*(1-xa)) + (x*xa)
	
	-- inputs
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	b.In.t = MTexture(1)
	b.In.tc = MTexCoord(1)
	
	-- 1-xa, 1-ya
	aa.In.x = a
	iaa.In.x = aa
	ba.In.x = b
	iba.In.x = ba
	
	-- (1-xa)*(1-ya)*materialAlpha
	mulAlpha.In.x = iaa
	mulAlpha.In.y = iba
	
	-- x*xa
	amul_a.In.x = a
	amul_a.In.y = aa
	
	-- (y*ya*(1-xa))
	mul3.In.x = b
	mul3.In.y = ba
	mul3.In.z = iaa
	
	-- (y*ya*(1-xa)) + (x*xa)
	add.In.x = mul3
	add.In.y = amul_a
	
	mulMat.In.x = add
	mulMat.In.y = MColor(0)
	sw_rgb.In.x = mulMat
	
	-- alpha output = (1-xa)*(1-ya)*materialAlpha
	invAlpha.In.x = mulAlpha
	mulMatAlpha.In.x = invAlpha
	mulMatAlpha.In.y = MColor(0)
	sw_alpha.In.x = mulMatAlpha -- swizzle w
	
	return { Default = { color = sw_rgb, alpha=sw_alpha } }
	
end

Compile(Shader())
