-- Main post processing shader.
-- Inputs (0 == framebuffer, 1 == colorize texture)
-- Controls: A = Bloom Scale, R = Colorize Scale

local Shader = function()

-- main
	local colorizeScale = Node("Sw_xxxx", "colorizeScale")
	colorizeScale.In.x = MColor(0)
	local bloomScale = Node("Sw_wwww", "bloomScale")
	bloomScale.In.x = MColor(0)
	
	local t = Node("SampleTexture2D", "t")
	t.In.t = MTexture(0)
	t.In.tc = MTexCoord(0)
	
-- bloom

	local bloomSample = Node("BloomSample", "bloomSample")
	local bloom = Node("Lerp", "bloom")
	
	bloomSample.In.t = MTexture(0)
	bloomSample.In.tc = MTexCoord(0)
	bloomSample.In.p = t
	
	bloom.In.a = t
	bloom.In.b = bloomSample
	bloom.In.t = bloomScale
	
-- colorize (inputs: bloom)
	local r = Node("SampleTexture2D", "r")
	local g = Node("SampleTexture2D", "g")
	local b = Node("SampleTexture2D", "b")
	local talpha = Node("Sw_w", "talpha")
	local sw_red = Node("Sw_xxxx", "red")
	local sw_green = Node("Sw_yyyy", "green")
	local sw_blue = Node("Sw_zzzz", "blue")
	local rTc = Node("Sw_x511", "rTc")
	local gTc = Node("Sw_x511", "gTc")
	local bTc = Node("Sw_x511", "bTc")
	local cin = Node("Swp_xyz1", "cin")
	local colorize = Node("Lerp", "colorize")
	local maskr = Node("Sw_x", "maskr")
	local maskg = Node("Sw_y", "maskg")
	local maskb = Node("Sw_z", "maskb")

	
	sw_red.In.x = bloom
	sw_green.In.x = bloom
	sw_blue.In.x = bloom
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
	
	cin.In.x = maskr
	cin.In.y = maskg
	cin.In.z = maskb
	
	colorize.In.a = bloom
	colorize.In.b = cin
	colorize.In.t = colorizeScale
	
	return { Default = { color = colorize } }

end

Compile(Shader())