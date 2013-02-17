-- HUD.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

HUD = Class:New()

function HUD.Spawn(self)
	
	HUD:Load()
	HUD:Layout()
	
	World.globalTimers:Add(
		function () HUD:Think() end,
		0
	)
end

function HUD.SetVisible(self, visible)
	self.widgets.Root:SetVisible(visible)
end

function HUD.Load(self)
	self.widgets = {}
	self.widgets.Root = UI:CreateRoot(UI.kLayer_HUD)
	
	self.gfx = {
		Arm = "UI/arm_button_M",
		ArmPressed = "UI/arm_button_pressed_M",
		ManipulateDisabled = "UI/manipulate_button_charging_M",
		ManipulateEnabled = "UI/manipulate_button_M",
		ManipulateFlashing = "UI/manipulate_button_flashing_M",
		RechargeShimmer = "UI/ability_recharge_shimmer_M",
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
		self.widgets.Root
	)
	
	self.widgets.Manipulate = UIPushButton:Create(
		UI:MaterialSize(self.gfx.ManipulateEnabled, {0, 0}),
		{ -- we go to disabled state when manipulate gets activated
			pressed = self.gfx.ManipulateEnabled, 
			enabled = self.gfx.ManipulateEnabled,
			disabled = self.gfx.ManipulateDisabled
		},
		nil,
		{pressed=function (widget) HUD:ManipulatePressed(widget) end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.ManipulateCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.ManipulateEnabled})
	self.widgets.Root:AddChild(self.widgets.ManipulateCharging)
	self.widgets.ManipulateCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.ManipulateCharging:SetVisible(false)
	
	self.widgets.ManipulateShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.Root:AddChild(self.widgets.ManipulateShimmer)
	self.widgets.ManipulateShimmer:SetVisible(false)
	
	self.widgets.Manipulate.flashing = false
--	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	
	self.widgets.Shield = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Shield, {0, 0}),
		{pressed = self.gfx.ShieldPressed, enabled = self.gfx.Shield},
		nil,
		{pressed=function (widget) HUD:ShieldPressed(widget) end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.Pulse = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Pulse, {0, 0}),
		{pressed = self.gfx.PulsePressed, enabled = self.gfx.Pulse},
		nil,
		{pressed=function (widget) HUD:PulsePressed(widget) end},
		nil,
		self.widgets.Root
	)
	
end

function HUD.ArmPressed(self, widget)
	COutLine(kC_Debug, "Arm Pressed!")
	Arm:Start("chat")
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

function HUD.Layout(self)
	local y = 42
	local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth, y)
	self.widgets.ManipulateCharging:SetRect(r)
	self.widgets.ManipulateShimmer:SetRect(r)
	y = y + r[4] + (24 * UI.identityScale[2])
	r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth, y)
	y = y + r[4] + (24 * UI.identityScale[2])
	r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth, y)
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
			gfx.disabled = self.gfx.ManipulateDisabled
		end
		
		self.widgets.Manipulate.class:ChangeGfx(self.widgets.Manipulate, gfx)
	end
end

function HUD.RechargeManipulate(self)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	self.widgets.ManipulateCharging:SetVisible(true)
	
	local rechargeTime = PlayerSkills:ManipulateRechargeTime()
	
	self.widgets.ManipulateCharging:FillCircleTo(0, 0)
	self.widgets.ManipulateCharging:FillCircleTo(1, rechargeTime)
	
	local f = function ()
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, true)
		HUD:Shimmer(self.widgets.ManipulateShimmer)
	end
	
	World.gameTimers:Add(f, rechargeTime, true)
end

function HUD.Shimmer(self, widget)
	widget:SetVisible(true)
	widget:BlendTo({0,0,0,0}, 0)
	widget:BlendTo({1,1,1,1}, 0.2)
	
	local f = function()
		widget:BlendTo({0,0,0,0}, 0.2)
	end
	
	World.gameTimers:Add(f, 0.2, true)
end
