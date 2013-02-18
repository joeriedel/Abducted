-- TerminalScreen.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TerminalScreen = Entity:New()
TerminalScreen.MaxTouchDistancePct = 1/8
TerminalScreen.TouchPosShift = {0,0,48}
TerminalScreen.ButtonPosShift = {1/10, 1/10}
TerminalScreen.ButtonSize = {1/15, 1/15}
TerminalScreen.PopupEntity = nil
TerminalScreen.Objects = {}
TerminalScreen.Widgets = {}

function TerminalScreen.Spawn(self)
	Entity.Spawn(self)
	
	self.model = LoadModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:ScaleTo(Vec3ForString(self.keys.scale, {1,1,1}), 0)

	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
	self.enabled = BoolForString(self.keys.enabled, false)
	self.active = false
	self.popup = false
	
	table.insert(TerminalScreen.Objects, self)
end

function TerminalScreen.CheckTouch(self, x, y)
	if (not (self.enabled and self.active)) then
		return false
	end
	
	return CheckWorldTouch(
		VecAdd(self:WorldPos(), TerminalScreen.TouchPosShift), 
		x, 
		y, 
		(TerminalScreen.MaxTouchDistancePct * UI.systemScreen.diagonal)
	)
end

function TerminalScreen.CheckProximity(self, playerPos)
	if (not self.enabled) then
		return false
	end
	
	local dd = VecSub(playerPos, self:WorldPos())
	dd = VecMag(dd)
	
	return (dd <= TerminalScreen.MaxActivateDistance)
end

function TerminalScreen.CheckActive(self, playerPos)

	local activate = self:CheckProximity(playerPos)
	self:Activate(activate)

end

function TerminalScreen.Activate(self, activate)

	if (self.active == activate) then
		return
	end
	
	self.active = activate

	if (not self.active) then
		if (self.popup) then
			TerminalScreen.CancelUI()
		end
	end
end

function TerminalScreen.Touched(self)
	if (self.popup) then
		return
	end
	
	self:PopupUI()
end

function TerminalScreen.OnEvent(self, cmd, args)

	if (cmd == "activate") then
		self:Activate(true)
	elseif (cmd == "deactivate") then
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
	if (TerminalScreen.Widgets.Hack.label) then
		TerminalScreen.Widgets.Hack.disableGfxChanges = false
		TerminalScreen.Widgets.Solve.disableGfxChanges = false
		TerminalScreen.Widgets.Hack.class:SetEnabled(TerminalScreen.Widgets.Hack, true)
		TerminalScreen.Widgets.Solve.class:SetEnabled(TerminalScreen.Widgets.Solve, true)
		TerminalScreen.Widgets.Hack.label:FadeTo({1,1,1,1}, 0.1)
		TerminalScreen.Widgets.Solve.label:Fadeto({1,1,1,1}, 0.1)
	end	
	
	local f = function ()
		TerminalScreen.PopupEntity = self
		self.popup = true
		TerminalScreen.Widgets.Hack:ScaleTo({1, 1}, {0.2,0.2})
		TerminalScreen.Widgets.Solve:ScaleTo({1, 1}, {0.2,0.2})
	end
	
	if (TerminalScreen.Widgets.Hack.label) then
		World.gameTimers:Add(f, 0.1, true)
	else
		f()
	end
end

function TerminalScreen.CancelUI(callback)
	if (TerminalScreen.PopupEntity) then
		if (TerminalScreen.Widgets.Hack.label) then
			TerminalScreen.Widgets.Hack.disableGfxChanges = true
			TerminalScreen.Widgets.Solve.disableGfxChanges = true
			TerminalScreen.Widgets.Hack.class:SetEnabled(TerminalScreen.Widgets.Hack, false)
			TerminalScreen.Widgets.Solve.class:SetEnabled(TerminalScreen.Widgets.Solve, false)
			TerminalScreen.Widgets.Hack.label:FadeTo({0,0,0,0}, 0.1)
			TerminalScreen.Widgets.Solve.label:Fadeto({0,0,0,0}, 0.1)
		end
		
		
		local f = function ()
			TerminalScreen.Widgets.Hack:ScaleTo({0, 0}, {0.2,0.2})
			TerminalScreen.Widgets.Solve:ScaleTo({0, 0}, {0.2,0.2})
			local f = function ()
				if (TerminalScreen.PopupEntity) then
					TerminalScreen.PopupEntity.popup = false
					TerminalScreen.PopupEntity = nil
				end
				if (callback) then
					callback()
				end
			end
			World.gameTimers:Add(f, 0.2, true)
		end
		
		if (TerminalScreen.Widgets.Hack.label) then
			World.gameTimers:Add(f, 0.1, true)
		else
			f()
		end
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
		local worldPos = TerminalScreen.PopupEntity:WorldPos()
		local p, r = World.Project(VecAdd(worldPos, TerminalScreen.TouchPosShift))
		p = UI:MapToUI(p)
		
		local shiftX = TerminalScreen.ButtonPosShift[1] * UI.identityScale[1] * UI.screenWidth
		local shiftY = TerminalScreen.ButtonPosShift[2] * UI.identityScale[2] * UI.screenHeight
		
		local wpos = {p[1]-shiftX, p[2]-shiftY}
		r = TerminalScreen.Widgets.Hack:Rect()
		r[1] = wpos[1] - r[3]
		r[2] = wpos[2] - r[4]
		BoundRect(r, TerminalScreen.ScreenBounds)
		TerminalScreen.Widgets.Hack:SetRect(r)
		
		wpos[1] = p[1] + shiftX
		wpos[2] = p[2] + shiftY
		r = TerminalScreen.Widgets.Solve:Rect()
		r[1] = wpos[1]
		r[2] = wpos[2]
		BoundRect(r, TerminalScreen.ScreenBounds)
		TerminalScreen.Widgets.Solve:SetRect(r)
	end
end

function TerminalScreen.Touch(e)

	if (not Input.IsTouchBegin(e)) then
		return false
	end

	local best
	local bestDist
	
	for k,v in pairs(TerminalScreen.Objects) do
		local r, dd = v:CheckTouch(e.data[1], e.data[2])
		if (r) then
			if ((bestDist==nil) or (dd < bestDist)) then
				best = v
				bestDist = dd
			end
		end
	end
	
	if (best) then
		e = UI:MapInputEvent(e)
		UI:AckFinger(e.data)
		best:Touched()
	end
	
	return (best ~= nil)
end

function TerminalScreen.CheckActivate(playerPos)

	for k,v in pairs(TerminalScreen.Objects) do
		v:CheckActive(playerPos)
	end

end

function TerminalScreen.HackPressed()
	Abducted.entity.eatInput = true -- during transition
	UI:BlendTo({1,1,1,1}, 0.3)
	TerminalPuzzles:InitGame("hack", 1, 1)
	
	local entity = TerminalScreen.PopupEntity
	TerminalScreen.CancelUI()
	
	local f = function ()
		UI:BlendTo({0,0,0,0}, 0.3)
		TerminalPuzzles:ShowBoard(true)
		
		local f = function ()
			local f = function(result)
				TerminalScreen.GameComplete(entity, result)
			end
			Abducted.entity.eatInput = false
			TerminalPuzzles:StartGame(f)
		end
		
		World.globalTimers:Add(f, 0.3, true)
	end
	
	World.globalTimers:Add(f, 0.3, true)
end

function TerminalScreen.SolvePressed()

end

function TerminalScreen.GameComplete(self, result)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	local f = function()
		Abducted.entity.eatInput = false
		UI:BlendTo({0,0,0,0}, 0.3)
		TerminalPuzzles:ShowBoard(false)
		TerminalPuzzles:ResetGame()
		collectgarbage()
	end
	World.globalTimers:Add(f, 0.3, true)
end

function TerminalScreen.StaticInit()

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
			enabled = UI.gfx.TerminalHack,
			pressed = UI.gfx.TerminalHackPressed
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed = TerminalScreen.HackPressed
		},
		nil,
		UI.widgets.interactive.Root
	)
	
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	TerminalScreen.Widgets.Hack = w
	
	w = UIPushButton:Create(
		rect,
		{
			enabled = UI.gfx.TerminalSolve,
			pressed = UI.gfx.TerminalSolvePressed
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed = TerminalScreen.SolvePressed
		},
		nil,
		UI.widgets.interactive.Root
	)
	
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	TerminalScreen.Widgets.Solve = w
	
	TerminalScreen.Widgets.Hack:ScaleTo({0, 0}, {0,0})
	TerminalScreen.Widgets.Solve:ScaleTo({0, 0}, {0,0})
	
	local bounds = 0.1
	
	TerminalScreen.ScreenBounds = {
		bounds * UI.screenWidth,
		bounds * UI.screenHeight,
		UI.screenWidth - (bounds * UI.screenWidth),
		UI.screenHeight - (bounds * UI.screenHeight)
	}
	
--[[	local w = UI:CreateStylePushButton(
		{0, 0, 8, 8},
		TerminalScreen.HackPressed,
		{ fontSize = "small" },
		UI.widgets.interactive.Root
	)
	
	local text = StringTable.Get("TERMINAL_HACK")
	UI:SetLabelText(w.label, text)
	local r = UI:SizeLabelToContents(w.label)
	local buttonRect = ExpandRect(
		r, 
		12 * UI.identityScale[1],
		12 * UI.identityScale[2]
	)
	
	w:SetRect(buttonRect)
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	TerminalScreen.Widgets.Hack = w
	
	w = UI:CreateStylePushButton(
		{0, 0, 8, 8},
		TerminalScreen.HackPressed,
		{ fontSize = "small" },
		UI.widgets.interactive.Root
	)
	
	text = StringTable.Get("TERMINAL_SOLVE")
	UI:SetLabelText(w.label, text)
	r = UI:SizeLabelToContents(w.label)
	buttonRect = ExpandRect(
		r, 
		12 * UI.identityScale[1],
		12 * UI.identityScale[2]
	)
	
	w:SetRect(buttonRect)
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	
	TerminalScreen.Widgets.Solve = w--]]
end

info_terminal = TerminalScreen
