-- Arm.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm = Class:New()
Arm.active = false

function Arm.Spawn(self)

	self:SpawnShared()
	self:SpawnChat()

end

-- 0, 448
-- 80, 528
-- = 80, 80
-- = 75, 75
-- = 0.03, 0.06
function Arm.SpawnShared(self)

	self.widgets = {}
	self.widgets.Root = UI:CreateRoot(UI.kLayer_Arm)
	self.widgets.Root.OnInputEvent = Arm.EatAllInput
	self.widgets.Root:SetVisible(false)

	self.gfx = {}
	self.gfx.Border = World.Load("UI/arm_screen1_M")
	self.gfx.LineBorder1 = World.Load("UI/lineborder1_M")
	self.gfx.LineBorder2 = World.Load("UI/lineborder2_M")
	self.gfx.LineBorder3 = World.Load("UI/lineborder3_M")
	self.gfx.Shimmer = World.Load("UI/shimmer1_M")
	self.gfx.ShimmerFlip = World.Load("UI/shimmer1_flip_M")
	self.gfx.Symbol = World.Load("UI/locked_symbol_M")
	self.gfx.SymbolFlash = World.Load("UI/locked_symbol_flash_M")
	
	self.sfx = {}
	self.sfx.ArmIntro = World.Load("Audio/armintro1")
	
	local xScale = UI.screenWidth / 1280
	local yScale = UI.screenHeight / 720
	
	-- the border is authored to be a 16:9 image packed in a square image, adjust for this
	
	local region = (1 - UI.yAspect) / 2
	local inset  = region * UI.screenWidth
	
	local rect = {0, -inset, UI.screenWidth, UI.screenHeight+inset*2}
	
	local wideRegion = (1 - (9/16)) / 2
	local wideInset = wideRegion * 1280 * xScale
	
	if (UI.systemScreen.aspect == "4x3") then
		wideInset = wideInset * 0.92 -- wtf?
	end
	
	self.screen = {
		1280 * 0.03906 * xScale,
		(720 * 0.06944 * yScale) + (wideInset-inset)*yScale,
		0,
		0
	}
	
	self.screen[3] = UI.screenWidth - (self.screen[1]*2)
	self.screen[4] = UI.screenHeight - (self.screen[2]*2)
	
	local size = self.gfx.LineBorder2:Dimensions()
	local w = size[1] * xScale
	local h = (size[2]/size[1]) * w
		
	self.workspaceLeft = {
		self.screen[1],
		self.screen[2],
		0,
		0
	}
	
	self.workspaceLeft[3] = UI.screenWidth -  (self.workspaceLeft[1]*2) - w
	self.workspaceLeft[4] = UI.screenHeight - (self.workspaceLeft[2]*2)
	
	self.workspaceRight = {
		self.workspaceLeft[1] + self.workspaceLeft[3],
		self.workspaceLeft[2],
		0,
		0
	}
	
	self.workspaceRight[3] = w
	self.workspaceRight[4] = h
	
	self.workspaceLeftSize = {
		0,
		0,
		self.workspaceLeft[3],
		self.workspaceLeft[4]
	}
	
	self.workspaceRightSize = {
		0,
		0,
		self.workspaceRight[3],
		self.workspaceRight[4]
	}
	
	self.widgets.Border = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.Border})
	self.widgets.Root:AddChild(self.widgets.Border)
	
	self.widgets.Shimmer = UI:CreateWidget("MatWidget", {rect=self.screen, material=self.gfx.Shimmer})
	self.widgets.Root:AddChild(self.widgets.Shimmer)
	
	self.widgets.LineBorder1 = UI:CreateWidget("MatWidget", {rect=self.workspaceLeft, material=self.gfx.LineBorder1})
	self.widgets.Root:AddChild(self.widgets.LineBorder1)
	
	self.widgets.LineBorder2 = UI:CreateWidget("MatWidget", {rect=self.workspaceRight, material=self.gfx.LineBorder2})
	self.widgets.Root:AddChild(self.widgets.LineBorder2)
	
	local symbolSize = UI:MaterialSize(self.gfx.Symbol)
	symbolSize[3] = UI.screenWidth * 0.40
	symbolSize[4] = symbolSize[3]
	
	self.widgets.WorkspaceLeft = UI:CreateWidget("Widget", {rect=self.workspaceLeft})
	self.widgets.Root:AddChild(self.widgets.WorkspaceLeft)
	
	self.widgets.Symbol = UI:CreateWidget("MatWidget", {rect=symbolSize, material=self.gfx.Symbol})
	self.widgets.WorkspaceLeft:AddChild(self.widgets.Symbol)	
	
end

function Arm.EatAllInput(self, e)
	return true
end

function Arm.ResetWidgets(self)
	self.widgets.Shimmer:BlendTo({0,0,0,0}, 0)
	self.widgets.Symbol:BlendTo({0,0,0,0}, 0)
	self.widgets.LineBorder1:BlendTo({0,0,0,0}, 0)
	self.widgets.LineBorder2:BlendTo({0,0,0,0}, 0)
end

function Arm.Start(self, mode)
	Abducted.entity.eatInput = true
	Arm.active = true
	
	self.mode = mode
	UI:BlendTo({1,1,1,1}, 0.2)
	self:ResetWidgets()
	
	self.sfx.ArmIntro:Rewind()
	self.sfx.ArmIntro:Play(kSoundChannel_UI, 0)
	
	local f = function()
		UI:BlendTo({1,1,1,0}, 0.2)
		HUD.widgets.Arm.class:Reset(HUD.widgets.Arm) -- eatInput we'll never get an up event for this
		World.DrawUIOnly(true) -- no world rendering anymore
		Arm:Intro()
	end
	
	World.globalTimers:Add(f, 0.2, true)
end

function Arm.Intro(self)
	self.widgets.Root:SetVisible(true)
	
	local f = function()
		Arm:DoShimmer()
	end
	
	World.globalTimers:Add(f, 0.5, true)
end

function Arm.DoShimmer(self)
	self.widgets.Shimmer:SetMaterial(self.gfx.Shimmer)
	self.widgets.Shimmer:SetHAlign(kHorizontalAlign_Left)
	self.widgets.Shimmer:SetVAlign(kVerticalAlign_Top)
	self.widgets.Shimmer:BlendTo({1,1,1,1}, 0)
	self.widgets.Shimmer:ScaleTo({0, 0}, {0, 0})
	self.widgets.Shimmer:ScaleTo({1, 1}, {0.15, 0.1})
	
	local f = function()
		self.widgets.Shimmer:SetMaterial(self.gfx.ShimmerFlip)
		self.widgets.Shimmer:SetHAlign(kHorizontalAlign_Right)
		self.widgets.Shimmer:SetVAlign(kVerticalAlign_Bottom)
		self.widgets.Shimmer:ScaleTo({0, 0}, {0.15, 0.1})
		Arm:DoInitialize()
	end
	
	World.globalTimers:Add(f, 0.1, true)
end

function Arm.DoInitialize(self)
	local r = UI:CenterWidget(self.widgets.Symbol, {0, 0, self.workspaceLeft[3], self.workspaceLeft[4]})
	local x = r[1]
	r[1] = r[1] + self.workspaceLeft[3] * 0.08
	self.widgets.Symbol:SetRect(r)
	self:ActivateSymbol(
		true,
		{
			flashMin = 2,
			flashMax = 2,
			singleShot = true
		}
	)
	
	self.widgets.Symbol:MoveTo({x, r[2]}, {3.5, 0})
	self.widgets.Symbol:BlendTo({1,1,1,1}, 0.5)
	
	self.widgets.LineBorder1:BlendTo({1,1,1,1}, 0)
	self.widgets.LineBorder1:ScaleTo({0, 0}, {0, 0})
	self.widgets.LineBorder1:ScaleTo({1, 1}, {1, 1})
	
	local f = function()
		Arm:TransitionChat()
	end
	
	World.globalTimers:Add(f, 2.8, true)
	
	f = function()
		Arm:ButtonsIntro()
	end
	
	World.globalTimers:Add(f, 1.0, true)
	
end

function Arm.TransitionChat(self)
	self:ActivateSymbol(
		true,
		{
			flashMin = 1,
			flashMin = 1,
			singleShot = true
		}
	)
	
	self.widgets.Symbol:BlendTo({0,0,0,0}, 0.5)
	
	local f = function()
		Arm:StartChat()
	end
	
	World.globalTimers:Add(f, 0.5, true)
end

function Arm.ActivateSymbol(self, active, options)

	self.symbolActive = active
	if (active) then
		self:ToggleSymbolFlash(true, options)
	end

end

function Arm.ToggleSymbolFlash(self, flash, options)
	if (not self.symbolActive) then
		return
	end
	
	if (flash) then
		self.widgets.Symbol:SetMaterial(self.gfx.SymbolFlash)
		local f = function ()
			Arm:ToggleSymbolFlash(false, options)
		end
		World.globalTimers:Add(f, FloatRand(options.flashMin, options.flashMax), true)
	else
		self.widgets.Symbol:SetMaterial(self.gfx.Symbol)
		if (not options.singleShot) then
			local f = function ()
				Arm:ToggleSymbolFlash(true, options)
			end
			World.globalTimers:Add(f, FloatRand(options.solidMin, options.solidMax), true)
		end
	end
end

function Arm.ButtonsIntro(self)
	self.widgets.LineBorder2:BlendTo({1,1,1,1}, 1.5)
	
	local f = function()
		Arm:ListButtons()
	end
	
	World.globalTimers:Add(f, 1.5, true)
end

function Arm.ListButtons(self, num)
	
end

arm_ui = Arm