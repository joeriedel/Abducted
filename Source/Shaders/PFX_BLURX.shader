-- BlurX shader

local Shader = function()
	local filter = Node("GaussX", "filter")
	filter.In.t = MTexture(0)
	filter.In.tc = MTexCoord(0)
	filter.In.pfx = MPFXVars(0)

	return { Default = { color = filter }  }

end

Compile(Shader())
