-- Flat Single Textured Pixel
-- Muls by Material Color
-- Alpha output

local Shader = function()
	local a = Node("SampleTexture2D", "Texture1")
	local mul = Node("Mul", "Mul")
	
	-- sample texture 0
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	-- mul by diffuse color
	mul.In.x = a
	mul.In.y = MColor(0)
	
	local Output = { color = mul }
	
	return { Default = Output }
end

Compile(Shader())
