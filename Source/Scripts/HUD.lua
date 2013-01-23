-- HUD.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

HUD = Class:New()

function HUD.Spawn(self)
	
	HUD:Load()
	if (UI.wideScreen) then
		HUD:LayoutWide()
	else
		HUD:Layout4x3()
	end
	
	World.globalTimers:Add(
		function () HUD:Think() end,
		0
	)
end

function HUD.Load(self)
	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_HUD)
	
	self.gfx = {
		Arm = "UI/arm_button_M",
		ArmPressed = "UI/arm_button_pressed_M",
		Manipulate = "UI/manipulate_button_M",
		ManipulatePressed = "UI/manipulate_button_pressed_M",
		Pulse = "UI/pulse_button_M",
		PulsePressed = "UI/pulse_button_pressed_M",
		Shield = "UI/shield_button_M",
		ShieldPressed = "UI/shield_button_pressed_M"
	}
	
	map(self.gfx, World.Load)
	
	self.widgets.Arm = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Arm, {0, 0}),
		{pressed = self.gfx.ArmPressed, unpressed = self.gfx.Arm},
		nil,
		{pressed=function (widget) HUD:ArmPressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Manipulate = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Manipulate, {0, 0}),
		{pressed = self.gfx.ManipulatePressed, unpressed = self.gfx.Manipulate},
		nil,
		{pressed=function (widget) HUD:ManipulatePressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Shield = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Shield, {0, 0}),
		{pressed = self.gfx.ShieldPressed, unpressed = self.gfx.Shield},
		nil,
		{pressed=function (widget) HUD:ShieldPressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Pulse = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Pulse, {0, 0}),
		{pressed = self.gfx.PulsePressed, unpressed = self.gfx.Pulse},
		nil,
		{pressed=function (widget) HUD:PulsePressed(widget) end},
		nil,
		self.widgets.root
	)
	
end

function HUD.ArmPressed(self, widget)
	COutLine(kC_Debug, "Arm Pressed!")
end

function HUD.ManipulatePressed(self, widget)
	COutLine(kC_Debug, "Manipulate Pressed!")
end
function HUD.ShieldPressed(self, widget)
	COutLine(kC_Debug, "Shield Pressed!")
end

function HUD.PulsePressed(self, widget)
	COutLine(kC_Debug, "Pulse Pressed!")
end

function HUD.Layout4x3(self)
	self:LayoutWide()
end

function HUD.LayoutWide(self)
	UI:MoveWidget(self.widgets.Manipulate, 1133, 42)
	UI:MoveWidget(self.widgets.Shield, 1133, 187)
	UI:MoveWidget(self.widgets.Pulse, 1133, 331)
end

function HUD.Think(self)
	
end
