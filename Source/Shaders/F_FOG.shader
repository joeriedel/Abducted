-- Fog Pixel

local Shader = function()

	SetPrecisionMode("medium")
	SetSamplerPrecision(0, "medium")

	local fog = Node("DepthFog", "fog")
	fog.In.t = MTexture(0)
	fog.In.tc = MTexCoord(0)
	fog.In.f = MColor(0)
	fog.In.pr = MProjection(0)

	return { Default = {color = fog}, Preview = {color = MColor(0)} }
end

Compile(Shader())
