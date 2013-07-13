-- PostFX.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PostFX = {}

PostFX.kBrightpass = 0
PostFX.kBlur = 1
PostFX.kNumBlurPasses = 4
PostFX.kBloom = 100

function PostFX.Init()

	PostFX.gfx = {}
	PostFX.gfx.Brightpass = World.Load("FX/Brightpass_M")
	PostFX.gfx.Bloom = World.Load("FX/Bloom_M")
	
	local fastBlur = false
	
	if (fastBlur) then
		PostFX.gfx.FastBlur = World.Load("FX/FastBlur_M")
	else
		PostFX.gfx.BlurX = World.Load("FX/BlurX_M")
		PostFX.gfx.BlurY = World.Load("FX/BlurY_M")
	end
	
	local scale = 0.3
	
	World.CreatePostProcessEffect(PostFX.kBrightpass, PostFX.gfx.Brightpass)
	World.SetPostProcessEffectScale(PostFX.kBrightpass, {scale, scale})
	World.EnablePostProcessEffect(PostFX.kBrightpass, true)
	
	for i=0,(PostFX.kNumBlurPasses-1) do
		if (fastBlur) then
			World.CreatePostProcessEffect(PostFX.kBlur+i, PostFX.gfx.FastBlur)
			World.SetPostProcessEffectScale(PostFX.kBlur+i, {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+i, true)
		else
			World.CreatePostProcessEffect(PostFX.kBlur+(i*2), PostFX.gfx.BlurX)
			World.SetPostProcessEffectScale(PostFX.kBlur+(i*2), {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2), true)
			
			World.CreatePostProcessEffect(PostFX.kBlur+(i*2)+1, PostFX.gfx.BlurY)
			World.SetPostProcessEffectScale(PostFX.kBlur+(i*2)+1, {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2)+1, true)
		end
	end
	
	World.CreatePostProcessEffect(PostFX.kBloom, PostFX.gfx.Bloom)
	World.SetPostProcessEffectScale(PostFX.kBloom, {scale, scale})
	World.EnablePostProcessEffect(PostFX.kBloom, true)

end