-- Performs brightpass filter

local Shader = function()

	local tex = Node("SampleTexture2D", "tex")
	tex.In.t = MTexture(0)
	tex.In.tc = MTexCoord(0)

	local filter = Node("BrightpassFilter", "filter")
	filter.In.x = tex

	return { Default = { color = filter }  }

end

Compile(Shader())
