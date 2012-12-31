-- Lerps between src and inv src by material alpha
-- A = control

local Shader = function()

	local lerp = Node("Lerp", "Lerp")
	local t = Node("SampleTexture2D", "t")
	local r = Node("SampleTexture2D", "r")
	local g = Node("SampleTexture2D", "g")
	local b = Node("SampleTexture2D", "b")
	local inv = Node("OneMinus", "inv")
	local alpha = Node("Sw_w", "alpha")
	local blend = Node("Sw_zzzz", "alpha")
	local talpha = Node("Sw_w", "talpha")
	local sw_red = Node("Sw_xxxx", "red")
	local sw_green = Node("Sw_yyyy", "green")
	local sw_blue = Node("Sw_zzzz", "blue")
	local rTc = Node("Sw_x511", "rTc")
	local gTc = Node("Sw_x511", "gTc")
	local bTc = Node("Sw_x511", "bTc")
	local tex = Node("Swp_xyz1", "tex")
	local maskr = Node("Sw_x", "maskr")
	local maskg = Node("Sw_y", "maskg")
	local maskb = Node("Sw_z", "maskb")
	local out = Node("Swp_xyzw_w", "out")
	
	blend.In.x = MColor(0)
	alpha.In.x = MColor(0)
	
	t.In.t = MTexture(0)
	t.In.tc = MTexCoord(0)
	
	inv.In.x = t
	talpha.In.x = t
	
	sw_red.In.x = t
	sw_green.In.x = t
	sw_blue.In.x = t
	rTc.In.x = sw_red
	gTc.In.x = sw_green
	bTc.In.x = sw_blue
	
	-- remap is in texture 1
	r.In.t = MTexture(1)
	r.In.tc = rTc
	g.In.t = MTexture(1)
	g.In.tc = gTc
	b.In.t = MTexture(1)
	b.In.tc = bTc
	
	maskr.In.x = r
	maskg.In.x = g
	maskb.In.x = b
	
	tex.In.x = maskr
	tex.In.y = maskg
	tex.In.z = maskb
	
	lerp.In.a = t
	lerp.In.b = tex
	lerp.In.t = blend
	
	out.In.xyzw = lerp
	out.In.w = alpha
	
	return { Default = { color = out } }

end

Compile(Shader())