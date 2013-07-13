-- Bloom shader

local Shader = function()

	local bloom = Node("SampleTexture2D", "bloom")
	bloom.In.t = MTexture(0)
	bloom.In.tc = MTexCoord(0)

	local framebuffer = Node("SampleTexture2D", "framebuffer")
	framebuffer.In.t = MTexture(1)
	framebuffer.In.tc = MTexCoord(1)

	local filter = Node("Bloom", "filter")
	filter.In.fb = framebuffer
	filter.In.bloom = bloom

	return { Default = { color = filter }  }

end

Compile(Shader())
