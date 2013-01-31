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
	self.gfx.Button = World.Load("UI/arm_buttons_M")
	self.gfx.ButtonOverbright = World.Load("UI/arm_buttons_overbright_M")
	self.gfx.Border = World.Load("UI/arm_screen1_M")
	self.gfx.LineBorder1 = World.Load("UI/lineborder1_M")
	self.gfx.LineBorder2 = World.Load("UI/lineborder2_M")
	self.gfx.LineBorder3 = World.Load("UI/lineborder3_M")
	self.gfx.Shimmer = World.Load("UI/shimmer1_M")
	self.gfx.ShimmerFlip = World.Load("UI/shimmer1_flip_M")
	self.gfx.Symbol = World.Load("UI/locked_symbol_M")
	self.gfx.SymbolFlash = World.Load("UI/locked_symbol_flash_M")
	
	self.typefaces = {}
	self.typefaces.StandardButton = World.Load("UI/StandardButton_TF")
	
	self.sfx = {}
	self.sfx.ArmIntro = World.Load("Audio/armintro1")
	self.sfx.Button = World.Load("Audio/armbutton")
	
	Arm:SetupBackgroundAndWorkspaces()
	Arm:CreateMenu()
end

function Arm.SetupBackgroundAndWorkspaces(self)
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
	
	local symbolSize = UI:MaterialSize(self.gfx.Symbol)
	symbolSize[3] = UI.screenWidth * 0.40
	symbolSize[4] = symbolSize[3]
	
	self.widgets.WorkspaceLeft = UI:CreateWidget("Widget", {rect=self.workspaceLeft})
	self.widgets.Root:AddChild(self.widgets.WorkspaceLeft)
	
	self.widgets.WorkspaceRight = UI:CreateWidget("Widget", {rect=self.workspaceRight})
	self.widgets.Root:AddChild(self.widgets.WorkspaceRight)
	
	self.widgets.Symbol = UI:CreateWidget("MatWidget", {rect=symbolSize, material=self.gfx.Symbol})
	self.widgets.WorkspaceLeft:AddChild(self.widgets.Symbol)
end

function Arm.CreateMenu(self)
	self.widgets.LineBorder2 = UI:CreateWidget("MatWidget", {rect=self.workspaceRightSize, material=self.gfx.LineBorder2})
	self.widgets.WorkspaceRight:AddChild(self.widgets.LineBorder2)
	self.widgets.LineBorder3 = UI:CreateWidget("MatWidget", {rect=self.workspaceRightSize, material=self.gfx.LineBorder3})
	self.widgets.WorkspaceRight:AddChild(self.widgets.LineBorder3)
	
	local size = self.gfx.LineBorder2:Dimensions()
	local scale = {
		self.workspaceRightSize[3] / size[1],
		self.workspaceRightSize[4] / size[2]
	}
	
	-- scale buttons accordingly:
	size = self.gfx.Button:Dimensions()
	
	size[1] = size[1] * scale[1]
	size[2] = size[2] * scale[2] * 0.95
	
	local x = 7 * scale[1]
	local y = 0
	
	local shift = 15 * scale[1]
	local space = 0 * UI.identityScale[2]
	local lineSpace = 4
	
	local text
	
	self.widgets.Return = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			enabled = self.gfx.Button,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.ReturnPressed
		},
		{
			highlight = {
				on = {0.5,0.5,0.5,1},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	text = StringTable.Get("ARM_RETURN_TO_WORLD_BTN")
	UI:LineWrapCenterText(self.widgets.Return.label, lineSpace, text)
	
	y = y + size[2] + space
	
	self.widgets.Database = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			enabled = self.gfx.Button,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.DatabasePressed
		},
		{
			highlight = {
				on = {0.5,0.5,0.5,1},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	text = StringTable.Get("ARM_DATABASE_BTN")
	UI:LineWrapCenterText(self.widgets.Database.label, lineSpace, text)
	
	y = y + size[2] + space
	
	self.widgets.Talk = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			enabled = self.gfx.Button,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.TalkPressed
		},
		{
			highlight = {
				on = {0.5,0.5,0.5,1},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	self.widgets.Talk.skipIntro = true
	text = StringTable.Get("ARM_TALK_BTN")
	UI:LineWrapCenterText(self.widgets.Talk.label, lineSpace, text)
	
	self.widgets.Change = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.ChangePressed
		},
		{
			highlight = {
				on = {0,0,0,0},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	self.widgets.Change.skipIntro = true
	text = StringTable.Get("ARM_CHANGE_CONVERSATION_BTN")
	UI:LineWrapCenterText(self.widgets.Change.label, lineSpace, text)
	
	y = y + size[2] + space
	
	self.widgets.Powers = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			enabled = self.gfx.Button,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.PowersPressed
		},
		{
			highlight = {
				on = {0.5,0.5,0.5,1},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	text = StringTable.Get("ARM_POWERS_BTN")
	UI:LineWrapCenterText(self.widgets.Powers.label, lineSpace, text)
	
	y = y + size[2] + space
	
	self.widgets.Quit = UIPushButton:Create(
		{x, y, size[1], size[2]},
		{
			enabled = self.gfx.Button,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.QuitPressed
		},
		{
			highlight = {
				on = {0.5,0.5,0.5,1},
				off = {0,0,0,0},
				overbright = {1,1,1,1},
				time = 0.1,
				overbrightTime = 0.1
			},
			label = {
				typeface = self.typefaces.StandardButton
			}
		},
		self.widgets.WorkspaceRight
	)
	
	text = StringTable.Get("ARM_SAVE_AND_QUIT_BTN")
	UI:LineWrapCenterText(self.widgets.Quit.label, lineSpace, text)
	
	self.widgets.MenuButtons = {
		self.widgets.Return,
		self.widgets.Database,
		self.widgets.Talk,
		self.widgets.Change,
		self.widgets.Powers,
		self.widgets.Quit
	}
end

function Arm.ReturnPressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.DatabasePressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.TalkPressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.ChangePressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.PowersPressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.QuitPressed(widget)
	Arm:ClearButtonHighlights(widget)
end

function Arm.ClearButtonHighlights(self, except)
	for k,v in pairs(self.widgets.MenuButtons) do
		if (v ~= except) then
			v.class:ResetHighlight(v)
		end
	end
end

function Arm.EatAllInput(self, e)
	return true
end

function Arm.ResetWidgets(self)
	self.widgets.Shimmer:BlendTo({0,0,0,0}, 0)
	self.widgets.Symbol:BlendTo({0,0,0,0}, 0)
	self.widgets.LineBorder1:BlendTo({0,0,0,0}, 0)
	self.widgets.LineBorder2:BlendTo({0,0,0,0}, 0)
	self.widgets.LineBorder3:BlendTo({0,0,0,0}, 0)
	
	for k,v in pairs(self.widgets.MenuButtons) do
		v:BlendTo({0,0,0,0}, 0)
	end
	
	Arm:ResetChat()
end

function Arm.Start(self, mode)
	Abducted.entity.eatInput = true
	self.active = true
	self.intro = true
	self.modeCleanup = nil
	self.mode = nil
	self.backToGame = false
	self.talk = false
	self.introMode = mode
	
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

function Arm.SwitchToChat(self)

	self.modeCleanup = function ()
		Arm:EndChat()
	end
	
	self:SwapToChange()
	self:StartChat()
end

function Arm.SwapToTalk(self)
	if (self.talk) then
		return
	end
	
	self.widgets.Talk.class:SetEnabled(self.widgets.Talk, true, true)
	self.widgets.Talk:FadeTo({1,1,1,1}, 0.2)
	self.widgets.LineBorder2:FadeTo({0,0,0,0}, 0.5)
	self.widgets.LineBorder3:FadeTo({1,1,1,1}, 0.5)
end

function Arm.SwapToChange(self)
	if (not self.talk) then
		return
	end
	
	self.talk = false
	self.widgets.Talk.class:SetEnabled(self.widgets.Talk, false, false)
	self.widgets.Talk:FadeTo({0,0,0,0}, 0.2)
	self.widgets.LineBorder2:FadeTo({1,1,1,1}, 0.5)
	self.widgets.LineBorder3:FadeTo({0,0,0,0}, 0.5)
end

function Arm.EnableChangeTopic(self, enable)
	if (enable) then
		self.widgets.Change.class:SetEnabled(self.widgets.Change, true, true)
		self.widgets.Change:BlendTo({1,1,1,1}, 0.2)
	else
		self.widgets.Change.class:SetEnabled(self.widgets.Change, false, false)
		self.widgets.Change:BlendTo({0,0,0,0}, 0.2)
	end
end

function Arm.SwitchMode(self, mode)

	if (self.mode == mode) then
		return
	end

	if (self.modeCleanup) then
		self.modeCleanup()
		return
	end
	
	self:NextMode(mode)
	
end

function Arm.NextMode(self, mode)
	self.mode = mode
	
	if (mode == "chat") then
		self:SwitchToChat()
	end
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
		Arm:ListButtons(1)
	end
	
	World.globalTimers:Add(f, 1.5, true)
end

function Arm.ListButtons(self, num)
	local didFade = false
	local widget = self.widgets.MenuButtons[num]
	if (widget) then
		if (not widget.skipIntro) then
			widget:BlendTo({1,1,1,1}, 0.3)
			didFade = true
		end
	end
	
	local f
	
	if (self.widgets.MenuButtons[num+1]) then
		f = function()
			Arm:ListButtons(num+1)
		end
	else
		f = function()
			Arm:IntroComplete()
		end
	end
	
	if (didFade) then
		World.globalTimers:Add(f, 0.3, true)
	else
		f()
	end
end

function Arm.IntroComplete(self)
	Abducted.entity.eatInput = false
	self:SwitchMode(self.introMode)
	self.introMode = nil
end

function Arm.EnableMenuButtons(self, enable)
	for k,v in pairs(self.widgets.MenuButtons) do
		v.class:SetEnabled(v, enable)
	end
end

arm_ui = Arm