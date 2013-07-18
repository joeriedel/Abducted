-- L_1T_UNIFIED_SHADOW.shader
-- Projected Shadow Shader

local Shader = function()

	local shadowTexProj = Node("SampleTexture2DProj", "ShadowTexture")

	shadowTexProj.In.t = MTexture(0)
	shadowTexProj.In.tc = MTexCoord(0)

	local smoothFunc = Node("SmoothShadow", "smoothFunc")
	smoothFunc.In.x = shadowTexProj

	local shadowAttn = Node("ShadowAttn", "LightAttenuation")
	shadowAttn.In.lightVertex = MLightVertex(0)
	shadowAttn.In.normal = MNormal(0)
	local mulByAttn = Node("Mul", "MulByAttn")
	mulByAttn.In.x = shadowAttn
	mulByAttn.In.y = smoothFunc

	--local mulByMaterialColor = Node("Mul", "MulByMaterialColor")
	--mulByMaterialColor.In.x = mulByAttn
	--mulByMaterialColor.In.y = MColor(0)

	local preview = Node("SampleTexture2D", "PreviewTexture")

	preview.In.t = MTexture(0)
	preview.In.tc = MTexCoord(0)

	return { Preview = { color = preview }, Default = { color = mulByAttn } }

end

Compile(Shader())