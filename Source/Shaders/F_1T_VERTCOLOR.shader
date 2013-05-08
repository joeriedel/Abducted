-- F_1T_VERTCOLOR.shader
-- Flat Single Textured Pixel
-- Muls by Material and Vertex Color
-- Alpha output

local Shader = function()
	
	local a = Node("SampleTexture2D", "Texture1")
	local mul = Node("Mul", "Mul")
	local mul2 = Node("Mul", "Mul2")
	
	-- sample texture 0
	a.In.t = MTexture(0)
	a.In.tc = MTexCoord(0)
	-- mul by diffuse color
	mul.In.x = a
	mul.In.y = MColor(0)
	-- mul by vertex color
	mul2.In.x = mul
	mul2.In.y = MVertexColor(0)
		
	return { Default = { color = mul2 } }
end

Compile(Shader())