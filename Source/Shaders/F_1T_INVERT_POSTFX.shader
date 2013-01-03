-- Lerps between src and inv src by material alpha
-- A = control

local Shader = function()

	local lerp = Node("Lerp", "Lerp")
	local tex = Node("SampleTexture2D", "SampleTexture2D")
	local inv = Node("OneMinus", "Invert")
	local alpha = Node("Sw_wwww", "Alpha")
	local mulDiffuse = Node("Mul", "MulDiffuse")
		
	alpha.In.x = MColor(0)
	tex.In.t = MTexture(0)
	tex.In.tc = MTexCoord(0)
	inv.In.x = tex
	lerp.In.a = tex
	lerp.In.b = inv
	lerp.In.t = alpha
	
	mulDiffuse.In.x = MColor(0)
	mulDiffuse.In.y = lerp
	
	return { Default = { color = mulDiffuse } }

end

Compile(Shader())