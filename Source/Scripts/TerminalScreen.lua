-- TerminalScreen.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TerminalScreen = Entity:New()
TerminalScreen.MaxTouchDistancePct = 1/8
TerminalScreen.TouchPosShift = {0,0,48}
TerminalScreen.ButtonPosShift = {1.5/10, 1/10}
TerminalScreen.ButtonSize = {1.5/15, 1.5/15}
TerminalScreen.PopupEntity = nil
TerminalScreen.Objects = {}
TerminalScreen.Widgets = {}
TerminalScreen.Skip = false
TerminalScreen.DT = 0
TerminalScreen.Active = nil

function TerminalScreen.Spawn(self)
	Entity.Spawn(self)
	MakeAnimatable(self)
	
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
	
	self.model = LoadModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	
	local scale = Vec3ForString(self.keys.scale, {1,1,1})
	self.model.dm:ScaleTo(scale, 0)

	self:SetMins({-200*scale[1], -200*scale[2], 0*scale[3]})
	self:SetMaxs({ 200*scale[1],  200*scale[2], 282*scale[3]})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.model.glow1 = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.glow1)
	self.model.glow1:ScaleTo(scale, 0)
	self.model.glow1:SetBounds(self:Mins(), self:Maxs())
	self.model.glow1:BlendTo({1,1,1,0}, 0)
	
	self.model.glow2 = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.glow2)
	self.model.glow2:ScaleTo(scale, 0)
	self.model.glow2:SetBounds(self:Mins(), self:Maxs())
	self.model.glow2:BlendTo({1,1,1,0}, 0)
	
	self.alienSkin1 = World.Load("Shared/alienskin1_M")
	self.terminalFlash1 = World.Load("Objects/terminalglow1_M")
	self.terminalFlash2 = World.Load("Objects/terminalglow2_M")
	
	self.model.glow1:ReplaceMaterial(self.alienSkin1, self.terminalFlash1)
	self.model.glow2:ReplaceMaterial(self.alienSkin1, self.terminalFlash2)
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
	self.enabled = BoolForString(self.keys.enabled, false)
	self.activateRadius = NumberForString(self.keys.activate_radius, 200)
	self.active = false
	self.popup = false
	self.size = "small"
	self.state = "none"
	
	self:PlayAnim("idle", self.model)
	
	table.insert(TerminalScreen.Objects, self)
end

function TerminalScreen.CheckProximity(self, playerPos)
	if (not self.enabled) then
		return false
	end
	
	local dd = VecSub(playerPos, self:WorldPos())
	dd = VecMag(dd)
	
	return (dd <= self.activateRadius)
end

function TerminalScreen.CheckActive(self, playerPos)

	local activate = self:CheckProximity(playerPos)
	self:Activate(activate)

end

function TerminalScreen.Activate(self, activate)

	if (self.active == activate) then
		if (activate) then
			self:UpdateActivated()
		end
		return
	end
	
	self.active = activate

	if (self.active) then
		self:BeginActivated()
	else
		self:EndActivated()
		
	end
end

function TerminalScreen.BeginActivated(self)

	if (self.size == "big") then
		self:PopupUI()
	else
		self.activateTime = Game.time
	end

end

function TerminalScreen.EndActivated(self)
	if (self.popup) then
		TerminalScreen.CancelUI()
	end
	
	self.model.glow1:BlendTo({1,1,1,0}, 0.1)
	self.model.glow2:BlendTo({1,1,1,0}, 0.1)
	
	if (self.size == "small") then
		self:PlayAnim("idle", self.model)
	end
	
end

function TerminalScreen.UpdateActivated(self)

	if (self.size == "big") then
		return
	end
	
	local dt = Game.time - self.activateTime
	
	if ((self.state == "none") and (dt > 1)) then
		self.state = "wiggle"
		self:PlayAnim("wiggle1", self.model)
	elseif ((self.state == "wiggle") and (dt > 3)) then
		self.state = "flash1"
		self.model.glow1:BlendTo({1,1,1,1}, 0.1)
	elseif ((self.state == "flash1") and (dt > 5)) then
		self.state = "flash2"
		self.model.glow2:BlendTo({1,1,1,1}, 0.1)
		self.model.glow1:BlendTo({1,1,1,0}, 0.1)
	elseif ((self.state == "flash2") and (dt > 7)) then
		self.state = "grow"
		self.model.glow2:BlendTo({1,1,1,0}, 2)
		self.size = "big"
		local f = function()
			if (self.active) then
				self:PopupUI()
			end
			self:PlayAnim("grown", self.model)
		end
		self:PlayAnim("grow", self.model).Seq(f)
	end
end

function TerminalScreen.OnEvent(self, cmd, args)
	COutLineEvent("TerminalScreen", cmd, args)
	
	if (cmd == "enable") then
		self.enabled = true
		return true
	elseif (cmd == "disable") then
		self.enabled = false
		self:Activate(false)
	end

end

function TerminalScreen.PopupUI(self)
	local f = function()
		TerminalScreen.ExecutePopup(self)
	end
	TerminalScreen.CancelUI(f) -- cancel any active UI
end

function TerminalScreen.ExecutePopup(self)
	TerminalScreen.PopupEntity = self
	self.popup = true
	TerminalScreen.Widgets.Glyph:ScaleTo({1, 1}, {0.2,0.2})
end

function TerminalScreen.CancelUI(callback)
	if (TerminalScreen.PopupEntity) then
		Abducted.entity.eatInput = true
		TerminalScreen.Widgets.Glyph:ScaleTo({0, 0}, {0.2,0.2})
		local f = function ()
			Abducted.entity.eatInput = false
			if (TerminalScreen.PopupEntity) then
				TerminalScreen.PopupEntity.popup = false
				TerminalScreen.PopupEntity = nil
			end
			if (callback) then
				callback()
			end
		end
		World.gameTimers:Add(f, 0.2, true)
	else
		if (callback) then
			callback()
		end
	end
end

function TerminalScreen.UpdateUI()
	if (World.paused) then
		return
	end
	if (TerminalScreen.PopupEntity) then
		local worldPos = World.playerPawn:WorldPos()
		local p, r = World.Project(VecAdd(worldPos, TerminalScreen.TouchPosShift))
		p = UI:MapToUI(p)
		
		local shiftX = TerminalScreen.ButtonPosShift[1] * UI.identityScale[1] * UI.screenWidth
		local shiftY = TerminalScreen.ButtonPosShift[2] * UI.identityScale[2] * UI.screenHeight
		
		local wpos = {p[1]+shiftX, p[2]-shiftY}
		r = TerminalScreen.Widgets.Glyph:Rect()
		r[1] = wpos[1] - r[3]
		r[2] = wpos[2] - r[4]
		BoundRect(r, TerminalScreen.ScreenBounds)
		TerminalScreen.Widgets.Glyph:SetRect(r)
	end
end

function TerminalScreen.CheckActivate(dt)

	TerminalScreen.DT = TerminalScreen.DT + dt
	if (TerminalScreen.DT < 0.25) then
		return
	end
	
	TerminalScreen.DT = TerminalScreen.DT - 0.25

	local playerPos = World.playerPawn:WorldPos()
	
	for k,v in pairs(TerminalScreen.Objects) do
		v:CheckActive(playerPos)
	end

end

function TerminalScreen.GlyphPressed()
	local target = TerminalScreen.PopupEntity
	World.playerPawn:Stop()
	TerminalScreen.Active = target
	local f = function()
		World.playerPawn:EnterTerminal(target)
	end
	TerminalScreen.CancelUI(f)
end

function TerminalScreen.SolvePressed()
	local f = function()
		World.playerPawn:EnterSolveGame(TerminalScreen.Active)
	end
	TerminalScreen.HideUI(f)
end

function TerminalScreen.HackPressed()
	local f = function()
		World.playerPawn:EnterHackGame(TerminalScreen.Active)
	end
	TerminalScreen.HideUI(f)
end

function TerminalScreen.ShowUI()

	TerminalScreen.Widgets.Hack:BlendTo({1,1,1,1}, 0.2)
	TerminalScreen.Widgets.Hack.bkg:BlendTo({0,0,0,1}, 0.2)
	TerminalScreen.Widgets.Solve:BlendTo({1,1,1,1}, 0.2)
	TerminalScreen.Widgets.Solve.bkg:BlendTo({0,0,0,1}, 0.2)

end

function TerminalScreen.HideUI(callback)

	TerminalScreen.Widgets.Hack:BlendTo({0,0,0,0}, 0.2)
	TerminalScreen.Widgets.Hack.bkg:BlendTo({0,0,0,0}, 0.2)
	TerminalScreen.Widgets.Solve:BlendTo({0,0,0,0}, 0.2)
	TerminalScreen.Widgets.Solve.bkg:BlendTo({0,0,0,0}, 0.2)
	
	if (callback) then
		World.gameTimers:Add(callback, 0.2, true)
	end

end

function TerminalScreen.DoHackGame()
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	ReflexGame:InitGame(1, 1)
	
	local entity = TerminalScreen.Active
		
	if (TerminalScreen.Skip) then
		TerminalScreen.GameComplete(entity, "hack", true)
	else
		local f = function ()
			UI:BlendTo({0,0,0,0}, 0.3)
			ReflexGame:ShowBoard(true)
			
			local f = function ()
				local f = function(result)
					TerminalScreen.GameComplete(entity, "hack", result)
				end
				Abducted.entity.eatInput = false
				ReflexGame:StartGame(f)
			end
			
			World.globalTimers:Add(f, 0.3, true)
		end
		
		World.globalTimers:Add(f, 0.3, true)
	end
end

function TerminalScreen.DoSolveGame()
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	ReflexGame:InitGame(1, 1)
	
	local entity = TerminalScreen.Active
		
	if (TerminalScreen.Skip) then
		TerminalScreen.GameComplete(entity, "solve", true)
	else
		local f = function ()
			UI:BlendTo({0,0,0,0}, 0.3)
			ReflexGame:ShowBoard(true)
			
			local f = function ()
				local f = function(result)
					TerminalScreen.GameComplete(entity, "solve", result)
				end
				Abducted.entity.eatInput = false
				ReflexGame:StartGame(f)
			end
			
			World.globalTimers:Add(f, 0.3, true)
		end
		
		World.globalTimers:Add(f, 0.3, true)
	end
end

function TerminalScreen.GameComplete(self, mode, result)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	local f = function()
		UI:BlendTo({0,0,0,0}, 0.3)
        ReflexGame:ShowBoard(false)
        ReflexGame:ResetGame()
        MemoryGame:ShowBoard(false)
        MemoryGame:ResetGame()
		collectgarbage()
	end
	World.globalTimers:Add(f, 0.3, true)
	
	if (mode == "hack") then
		World.playerPawn:LeaveHackGame(self, result)
	else
		World.playerPawn:LeaveSolveGame(self, result)
	end
end

function TerminalScreen.PostHackEvents(self, result)

	if (result) then
		if (self.keys.hack_success) then
			World.PostEvent(self.keys.hack_success)
		end
	else
		if (self.keys.hack_fail) then
			World.PostEvent(self.keys.hack_fail)
		end
	end
		
	if (result) then
		if (self.keys.success) then
			World.PostEvent(self.keys.success)
		end
	else
		if (self.keys.fail) then
			World.PostEvent(self.keys.fail)
		end
	end

end

function TerminalScreen.PostSolveEvents(self, result)

	if (result) then
		if (self.keys.solve_success) then
			World.PostEvent(self.keys.solve_success)
		end
	else
		if (self.keys.solve_fail) then
			World.PostEvent(self.keys.solve_fail)
		end
	end

	if (result) then
		if (self.keys.success) then
			World.PostEvent(self.keys.success)
		end
	else
		if (self.keys.fail) then
			World.PostEvent(self.keys.fail)
		end
	end

end

function TerminalScreen.StaticInit()

	local typeface = World.Load("UI/TerminalScreenButtons_TF")
		
	-- Create buttons
	local rect = {
		0,
		0,
		TerminalScreen.ButtonSize[1] * UI.identityScale[1] * UI.screenWidth,
		TerminalScreen.ButtonSize[1] * UI.identityScale[1] * UI.screenWidth -- make it square
	}
	
	local w = UIPushButton:Create(
		rect,
		{
			enabled = UI.gfx.AnimatedGlpyh,
			pressed = UI.gfx.AnimatedGlpyhPressed
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed = TerminalScreen.GlyphPressed
		},
		nil,
		UI.widgets.interactive.Root
	)
	
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	w:ScaleTo({0,0}, {0,0})
	TerminalScreen.Widgets.Glyph = w
		
	local bounds = 0.1
	
	TerminalScreen.ScreenBounds = {
		bounds * UI.screenWidth,
		bounds * UI.screenHeight,
		UI.screenWidth - (bounds * UI.screenWidth),
		UI.screenHeight - (bounds * UI.screenHeight)
	}
	
	local w = UI:CreateStylePushButton(
		{0, 0, 8, 8},
		TerminalScreen.SolvePressed,
		{ typeface=typeface, highlight={on={0,0,0,0}} }
	)
	
	local text = StringTable.Get("TERMINAL_SOLVE")
	UI:SetLabelText(w.label, text)
	local r = UI:SizeLabelToContents(w.label)
	local buttonRect = ExpandRect(
		r, 
		12 * UI.identityScale[1],
		12 * UI.identityScale[2]
	)
	
	buttonRect[1] = UI.screenWidth * 0.46
	buttonRect[2] = UI.screenHeight * 0.27
	
	w:SetRect(buttonRect)
	w.highlight:SetRect({0,0,buttonRect[3], buttonRect[4]})
	UI:CenterLabel(w.label, {0,0,buttonRect[3], buttonRect[4]})
	w:BlendTo({0,0,0,0}, 0)
	TerminalScreen.Widgets.Solve = w
	
	w = UI:CreateWidget("MatWidget", {rect=buttonRect, material=UI.gfx.Solid})
	w:BlendTo({0,0,0,0}, 0)
	w:SetBlendWithParent(true)
	UI.widgets.interactive.Root:AddChild(w)
	UI.widgets.interactive.Root:AddChild(TerminalScreen.Widgets.Solve)
	TerminalScreen.Widgets.Solve.bkg = w
	
	w = UI:CreateStylePushButton(
		{0, 0, 8, 8},
		TerminalScreen.HackPressed,
		{ typeface=typeface, highlight={on={0,0,0,0}} }
	)
	
	text = StringTable.Get("TERMINAL_HACK")
	UI:SetLabelText(w.label, text)
	r = UI:SizeLabelToContents(w.label)
	buttonRect = ExpandRect(
		r, 
		12 * UI.identityScale[1],
		12 * UI.identityScale[2]
	)
	
	buttonRect[1] = UI.screenWidth * 0.41
	buttonRect[2] = UI.screenHeight * 0.65
	
	w:SetRect(buttonRect)
	w.highlight:SetRect({0,0,buttonRect[3], buttonRect[4]})
	UI:CenterLabel(w.label, {0,0,buttonRect[3], buttonRect[4]})
	w:BlendTo({0,0,0,0}, 0)
	TerminalScreen.Widgets.Hack = w
	
	w = UI:CreateWidget("MatWidget", {rect=buttonRect, material=UI.gfx.Solid})
	w:BlendTo({0,0,0,0}, 0)
	w:SetBlendWithParent(true)
	UI.widgets.interactive.Root:AddChild(w)
	UI.widgets.interactive.Root:AddChild(TerminalScreen.Widgets.Hack)
	TerminalScreen.Widgets.Hack.bkg = w
end


info_terminal = TerminalScreen
