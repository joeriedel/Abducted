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
--		ManipulateDisabled = "UI/manipulate_button_disabled_M",
		ManipulateEnabled = "UI/manipulate_button_M",
		ManipulateFlashing = "UI/manipulate_button_flashing_M",
		Pulse = "UI/pulse_button_M",
		PulsePressed = "UI/pulse_button_pressed_M",
		Shield = "UI/shield_button_M",
		ShieldPressed = "UI/shield_button_pressed_M"
	}
	
	map(self.gfx, World.Load)
	
	self.widgets.Arm = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Arm, {0, 0}),
		{pressed = self.gfx.ArmPressed, enabled = self.gfx.Arm},
		nil,
		{pressed=function (widget) HUD:ArmPressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Manipulate = UIPushButton:Create(
		UI:MaterialSize(self.gfx.ManipulateEnabled, {0, 0}),
		{ -- we go to disabled state when manipulate gets activated
			pressed = self.gfx.ManipulateEnabled, 
			enabled = self.gfx.ManipulateEnabled
		},
		nil,
		{pressed=function (widget) HUD:ManipulatePressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Manipulate.flashing = false
--	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	
	self.widgets.Shield = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Shield, {0, 0}),
		{pressed = self.gfx.ShieldPressed, enabled = self.gfx.Shield},
		nil,
		{pressed=function (widget) HUD:ShieldPressed(widget) end},
		nil,
		self.widgets.root
	)
	
	self.widgets.Pulse = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Pulse, {0, 0}),
		{pressed = self.gfx.PulsePressed, enabled = self.gfx.Pulse},
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
	Game.entity:BeginManipulate()
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
	self:UpdateManipulateButton()
end

function HUD.UpdateManipulateButton(self)
	local flashing = Game.entity.manipulate
	if (flashing ~= self.widgets.Manipulate.flashing) then
		self.widgets.Manipulate.flashing = flashing
		local gfx = {}
		if (flashing) then
			gfx.pressed = self.gfx.ManipulateFlashing
			gfx.enabled = self.gfx.ManipulateFlashing
		else
			gfx.pressed = self.gfx.ManipulateEnabled
			gfx.enabled = self.gfx.ManipulateEnabled
		end
		
		self.widgets.Manipulate.class:ChangeGfx(self.widgets.Manipulate, gfx)
	end
end
