-- F_2T_ADD_LUMSCALE.sh
-- Add two textures together, the second one is scaled by the luminosity of the first

-- Note:
-- Material.keys in the Packages folder contains a list of all the valid
-- shader names, if you add a new shader you must modify the Material.keys files
-- before you will see it in the list.

local Shader = function()

	-- First argument to Node() is the type of Node
	-- Do not reuse instances of a node, make a new one
	-- for each operation
	--
	-- There are nodes to multiply, add, or swizzle
	--
	-- Nodes are very easy to add, see the Nodes folder
	
	-- Second argument is a textual description of the node
	
	local diffuse = Node("SampleTexture2D", "Texture1")
	local diffuse2 = Node("SampleTexture2D", "Texture2")
	local addTextures = Node("Add", "Add Textures")
	local mulByMaterialColor = Node("Mul", "Multiply By Material Color")
	
	-- MTexture and MTexCoord both are used to address material input channel
	-- MTexture(N) gets texture N
	-- MTexCoord(N) gets UV channel N
	
	diffuse.In.t = MTexture(0) -- diffuse is first texture (i.e. Material.Texture1.Source)
	diffuse.In.tc = MTexCoord(0) -- coordinates come from first UV channel
	diffuse2.In.t = MTexture(1) -- lightmap is second texture (i.e. Material.Texture2.Source)
	diffuse2.In.tc = MTexCoord(1) -- coordinates come from second UV channel
	
	local lum = Node("Luminosity", "lum")
	lum.In.x = diffuse

	local lumMul = Node("Mul", "lumMul")
	lumMul.In.x = lum
	lumMul.In.y = diffuse2

	-- Add node does x + y
	-- So this does diffuse + diffuse2
	addTextures.In.x = diffuse
	addTextures.In.y = lumMul
	
	-- Multiply by the materials diffuse color
	mulByMaterialColor.In.x = addTextures -- take output from diffuse+diffuse2
	mulByMaterialColor.In.y = MColor(0)
	
	-- Output final value
	return { Default = { color = mulByMaterialColor } }
	
end

Compile(Shader())
