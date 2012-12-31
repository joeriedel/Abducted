-- Solid Color Pixel
-- Uses Materials Color
-- Alpha channel comes from texture

local Shader = function()
	local tex = Node("SampleTexture2D", "SampleTexture2D")
	local mul = Node("Mul", "Mul")
	local rgb = Node("Sw_xyz", "MaskRGB")
	local alpha = Node("Sw_w", "MaskAlpha")
	
	tex.In.t = MTexture(0)
	tex.In.tc = MTexCoord(0)
	
	mul.In.x = tex
	mul.In.y = MColor(0)
	
	rgb.In.x = MColor(0)
	alpha.In.x = mul
	
	return { Default = { color = rgb, alpha = alpha } }
end

Compile(Shader())
