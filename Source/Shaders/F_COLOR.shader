-- Solid Color Pixel
-- Uses Materials Color

local Shader = function()
	local Output = { color = MColor(0) }
	return { Default = Output }
end

Compile(Shader())
