-- Bloom shader
-- A = control

local Shader = function()

	local bloom = Node("BloomSample", "bloom")
	local lerp = Node("Lerp", "lerp")
	local tex = Node("SampleTexture2D", "tex")
	local blend = Node("Sw_wwww", "blend")
	
	blend.In.x = MColor(0)
	
	tex.In.t = MTexture(0)
	tex.In.tc = MTexCoord(0)
	
	bloom.In.t = MTexture(0)
	bloom.In.tc = MTexCoord(0)
	bloom.In.p = tex
	
	lerp.In.a = tex
	lerp.In.b = bloom
	lerp.In.t = blend
	
	return { Default = { color = lerp } }

end

Compile(Shader())
