-- PostFX.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PostFX = {}

PostFX.kBrightpass = 0
PostFX.kBlur = 1
PostFX.kNumBlurPasses = 2
PostFX.kBloom = 100
PostFX.kFastBlur = false
PostFX.BloomEnable = true

function PostFX.Init()

	PostFX.gfx = {}
	PostFX.gfx.Brightpass = World.Load("FX/Brightpass_M")
	PostFX.gfx.Bloom = World.Load("FX/Bloom_M")
	
	if (PostFX.kFastBlur) then
		PostFX.gfx.FastBlur = World.Load("FX/FastBlur_M")
	else
		PostFX.gfx.BlurX = World.Load("FX/BlurX_M")
		PostFX.gfx.BlurY = World.Load("FX/BlurY_M")
	end
	
	local scale = 0.25
	
	World.CreatePostProcessEffect(PostFX.kBrightpass, PostFX.gfx.Brightpass)
	World.SetPostProcessEffectScale(PostFX.kBrightpass, {scale, scale})
	World.EnablePostProcessEffect(PostFX.kBrightpass, PostFX.BloomEnable)
	
	for i=0,(PostFX.kNumBlurPasses-1) do
		if (PostFX.kFastBlur) then
			World.CreatePostProcessEffect(PostFX.kBlur+i, PostFX.gfx.FastBlur)
			World.SetPostProcessEffectScale(PostFX.kBlur+i, {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+i, PostFX.BloomEnable)
		else
			World.CreatePostProcessEffect(PostFX.kBlur+(i*2), PostFX.gfx.BlurX)
			World.SetPostProcessEffectScale(PostFX.kBlur+(i*2), {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2), PostFX.BloomEnable)
			
			World.CreatePostProcessEffect(PostFX.kBlur+(i*2)+1, PostFX.gfx.BlurY)
			World.SetPostProcessEffectScale(PostFX.kBlur+(i*2)+1, {scale, scale})
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2)+1, PostFX.BloomEnable)
		end
	end

	World.CreatePostProcessEffect(PostFX.kBloom, PostFX.gfx.Bloom)
	World.SetPostProcessEffectScale(PostFX.kBloom, {scale, scale})
	World.EnablePostProcessEffect(PostFX.kBloom, PostFX.BloomEnable)

end

cv_postfx_bloom = CVarFunc("r_togglebloom", "_cv_postfx_blur_func")

function _cv_postfx_blur_func()

	PostFX.BloomEnable = not PostFX.BloomEnable
	
	if (PostFX.BloomEnable) then
		COutLine(kC_Info, "Bloom ON")
	else
		COutLine(kC_Info, "Bloom OFF")
	end
	
	World.EnablePostProcessEffect(PostFX.kBrightpass, PostFX.BloomEnable)
	World.EnablePostProcessEffect(PostFX.kBloom, PostFX.BloomEnable)
	
	for i=0,(PostFX.kNumBlurPasses-1) do
		if (PostFX.kFastBlur) then
			World.EnablePostProcessEffect(PostFX.kBlur+i, PostFX.BloomEnable)
		else
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2), PostFX.BloomEnable)
			World.EnablePostProcessEffect(PostFX.kBlur+(i*2)+1, PostFX.BloomEnable)
		end
	end
	
end