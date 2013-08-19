-- ManipulatableObjectUI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ManipulatableObjectUI = Class:New()
ManipulatableObjectUI.MaxObjects = 5
ManipulatableObjectUI.Widgets = {}

function ManipulatableObjectUI.Spawn(self)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
	self.gfx = {}
	self.gfx.Center = World.Load("UI/generic_icon_base_M")
	self.gfx.CenterPressed = World.Load("UI/generic_icon_base_pressed_M")
	self.gfx.RightArrow = World.Load("UI/right_arrow_M")
	self.gfx.RightArrowPressed = World.Load("UI/right_arrow_pressed_M")
	
	self.actionTable = {
		kAction_Select1,
		kAction_Select2,
		kAction_Select3,
		kAction_Select4,
		kAction_Select5
	}
	
	self:MakePanels()
end

function ManipulatableObjectUI.MakePanels(self)

	for k,v in pairs(ManipulatableObjectUI.Widgets) do
		self:UnmapPanel(v)
	end
	
	ManipulatableObjectUI.Widgets = {}

	for i = 1,ManipulatableObjectUI.MaxObjects do
		local key = Abducted.KeyBindings.ActionToKey[self.actionTable[i]]
		local w = self:MakePanel(key)
		w.action = self.actionTable[i]
		ManipulatableObjectUI.Widgets[w.action] = w
	end
	
	collectgarbage()
end

function ManipulatableObjectUI.Notify(self, enabled)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
	self.nextPanel = 1
	self.target = nil
	
	if (enabled) then
		for k,v in pairs(ManipulatableObjectUI.Widgets) do
			v.target = nil
			v.circle:SetMaterial(self.gfx.Center)
		end
		self.enabled = true
	else
		for k,v in pairs(ManipulatableObjectUI.Widgets) do
			for z,w in pairs(v.widgets) do	
				if (w.class) then	
				-- reset pushbuttons
					w.class:Reset(w)
				end
			end
			v:BlendTo({1,1,1,0}, 0.3)
		end
		
		local f = function()
			self.enabled = false
		end
		
		World.globalTimers:Add(f, 0.3)
	end
		
end

function ManipulatableObjectUI.NotifyObject(self, entity, enabled, time)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
	if (not enabled) then
		return
	end
	
	local w = self.actionTable[self.nextPanel]
	if (w == nil) then
		return
	end
	self.nextPanel = self.nextPanel + 1
	
	w = ManipulatableObjectUI.Widgets[w]
	w.used = false
	w:BlendTo({1,1,1,0}, 0)
	w:SetVisible(true)
	w:BlendTo({1,1,1,1}, 0.3)
	
	if (entity:CanManipulateDir("left")) then
		w.left:SetVisible(true)
		w.left.bkg:SetVisible(true)
		w.left.bkg:BlendTo({1,1,1,0}, 0)
		w.left.label:SetVisible(true)
		w.left.label:BlendTo({1,1,1,0}, 0)
	else
		w.left:SetVisible(false)
		w.left.bkg:SetVisible(false)
		w.left.label:SetVisible(false)
	end
	
	if (entity:CanManipulateDir("right")) then
		w.right:SetVisible(true)
		w.right.bkg:SetVisible(true)
		w.right.bkg:BlendTo({1,1,1,0}, 0)
		w.right.label:SetVisible(true)
		w.right.label:BlendTo({1,1,1,0}, 0)
	else
		w.right:SetVisible(false)
		w.right.bkg:SetVisible(false)
		w.right.label:SetVisible(false)
	end
	
	if (entity:CanManipulateDir("up")) then
		w.up:SetVisible(true)
		w.up.bkg:SetVisible(true)
		w.up.bkg:BlendTo({1,1,1,0}, 0)
		w.up.label:SetVisible(true)
		w.up.label:BlendTo({1,1,1,0}, 0)
	else
		w.up:SetVisible(false)
		w.up.bkg:SetVisible(false)
		w.up.label:SetVisible(false)
	end
	
	if (entity:CanManipulateDir("down")) then
		w.down:SetVisible(true)
		w.down.bkg:SetVisible(true)
		w.down.bkg:BlendTo({1,1,1,0}, 0)
		w.down.label:SetVisible(true)
		w.down.label:BlendTo({1,1,1,0}, 0)
	else
		w.down:SetVisible(false)
		w.down.bkg:SetVisible(false)
		w.down.label:SetVisible(false)
	end
	
	w.target = entity
end

function ManipulatableObjectUI.NotifySingle(self)

	-- only one object?
	if (self.nextPanel == 2) then
		local w = ManipulatableObjectUI.Widgets[kAction_Select1]
		-- select it by default
		if (w) then
			self.target = w
			w.circle:SetMaterial(self.gfx.CenterPressed)
			self:ShowTarget(w, true)
		end
	end

end

function ManipulatableObjectUI.HandleAction(self, action)
	
	if ((action ~= kAction_ManipulateLeft) and (action ~= kAction_ManipulateRight) and
        (action ~= kAction_ManipulateUp) and (action ~= kAction_ManipulateDown) and
		(action ~= kAction_Select1) and (action ~= kAction_Select2) and
        (action ~= kAction_Select3) and (action ~= kAction_Select4) and
		(action ~= kAction_Select5)) then
		return false
	end
	
	if (self.target) then
		if (action == kAction_ManipulateLeft) then
			if (self.target.target:CanManipulateDir("left")) then
				UI.sfx.Command2:Play(kSoundChannel_UI, 0)
				self:ShowTarget(self.target, false)
				self.target.left.pressed = true
				self.target.left.class:SetGfxState(self.target.left, true)
				self.target.target:DoManipulateCommand("left")
				self.target = nil
			end
		elseif (action == kAction_ManipulateRight) then
			if (self.target.target:CanManipulateDir("right")) then
				UI.sfx.Command2:Play(kSoundChannel_UI, 0)
				self:ShowTarget(self.target, false)
				self.target.right.pressed = true
				self.target.right.class:SetGfxState(self.target.right, true)
				self.target.target:DoManipulateCommand("right")
				self.target = nil
			end
		elseif (action == kAction_ManipulateUp) then
			if (self.target.target:CanManipulateDir("up")) then
				UI.sfx.Command2:Play(kSoundChannel_UI, 0)
				self:ShowTarget(self.target, false)
				self.target.up.pressed = true
				self.target.up.class:SetGfxState(self.target.up, true)
				self.target.target:DoManipulateCommand("up")
				self.target = nil
			end
		elseif (action == kAction_ManipulateDown) then
			if (self.target.target:CanManipulateDir("down")) then
				UI.sfx.Command2:Play(kSoundChannel_UI, 0)
				self:ShowTarget(self.target, false)
				self.target.down.pressed = true
				self.target.down.class:SetGfxState(self.target.down, true)
				self.target.target:DoManipulateCommand("down")
				self.target = nil
			end
		end
	else
		local w = ManipulatableObjectUI.Widgets[action]
		if (w and (not w.used)) then
			self.target = w
			w.used = true
			w.circle:SetMaterial(self.gfx.CenterPressed)
			UI.sfx.Command:Play(kSoundChannel_UI, 0)
			self:ShowTarget(w, true)
		end
	end
	
	return true
end

function ManipulatableObjectUI.ShowTarget(self, w, show)
	local v = nil
	if (show) then
		v = {1,1,1,1}
	else
		v = {1,1,1,0}
	end
	
	w.left.bkg:BlendTo(v, 0.1)
	w.left.label:BlendTo(v, 0.1)
	w.right.bkg:BlendTo(v, 0.1)
	w.right.label:BlendTo(v, 0.1)
	w.up.bkg:BlendTo(v, 0.1)
	w.up.label:BlendTo(v, 0.1)
	w.down.bkg:BlendTo(v, 0.1)
	w.down.label:BlendTo(v, 0.1)
end

function ManipulatableObjectUI.UpdateUI(self)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
	if (not self.enabled) then
		return
	end
	
	for k,v in pairs(ManipulatableObjectUI.Widgets) do
		if (v.target) then
			local worldPos = VecAdd(v.target.manipulateTarget:WorldPos(), v.target.manipulateShift)
			local p,r = World.Project(worldPos)
			
			if (r) then
				p = UI:MapToUI(p)
				local r = MoveRectByCenter(v:Rect(), p[1], p[2])
				BoundRect(r, TerminalScreen.ScreenBounds)
				v:SetRect(r)
			end
		end
	end
end

function ManipulatableObjectUI.MakePanel(self, key)
	
	local panel = UI:CreateWidget("Widget", {rect={0,0,8,8}})
	panel.widgets = {}
	UI.widgets.interactive.Root:AddChild(panel)
	
	local circle = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=self.gfx.Center})
	circle:SetBlendWithParent(true)
	panel:AddChild(circle)
	table.insert(panel.widgets, circle)
	panel.circle = circle
	
	local label = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
	label:SetBlendWithParent(true)
	panel:AddChild(label)
	table.insert(panel.widgets, label)
	
	local arrowSize = 32*UI.identityScale[1]
	
	local left = UIPushButton:Create(
		{0,0,arrowSize,arrowSize},
		{
			enabled=self.gfx.RightArrow,
			pressed=self.gfx.RightArrowPressed
		},
		{ pressed = UI.sfx.Command2 },
		{pressed=function(widget) ManipulatableObjectUI:ArrowPressed(panel, "left") end},
		nil,
		panel
	)
	
	left:RotateTo({arrowSize/2, arrowSize/2, 180}, {0,0,0})
	
	left:SetBlendWithParent(true)
	table.insert(panel.widgets, left)
	panel.left = left
	
	local leftLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
	leftLabel.bkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
	panel:AddChild(leftLabel.bkg)
	panel:AddChild(leftLabel)
	panel.left.label = leftLabel
	panel.left.bkg = leftLabel.bkg
	leftLabel:SetBlendWithParent(true)
	leftLabel.bkg:SetBlendWithParent(true)
	
	local right = UIPushButton:Create(
		{0,0,arrowSize,arrowSize},
		{
			enabled=self.gfx.RightArrow,
			pressed=self.gfx.RightArrowPressed
		},
		{ pressed = UI.sfx.Command2 },
		{pressed=function(widget) ManipulatableObjectUI:ArrowPressed(panel, "right") end},
		nil,
		panel
	)
	right:SetBlendWithParent(true)
	table.insert(panel.widgets, right)
	panel.right = right
	
	local rightLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
	rightLabel.bkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
	panel:AddChild(rightLabel.bkg)
	panel:AddChild(rightLabel)
	panel.right.label = rightLabel
	panel.right.bkg = rightLabel.bkg
	rightLabel:SetBlendWithParent(true)
	rightLabel.bkg:SetBlendWithParent(true)
	
	local up = UIPushButton:Create(
		{0,0,arrowSize,arrowSize},
		{
			enabled=self.gfx.RightArrow,
			pressed=self.gfx.RightArrowPressed
		},
		{ pressed = UI.sfx.Command2 },
		{pressed=function(widget) ManipulatableObjectUI:ArrowPressed(panel, "up") end},
		nil,
		panel
	)
	up:RotateTo({arrowSize/2, arrowSize/2, -90}, {0,0,0})
	up:SetBlendWithParent(true)
	table.insert(panel.widgets, up)
	panel.up = up
	
	local upLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
	upLabel.bkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
	panel:AddChild(upLabel.bkg)
	panel:AddChild(upLabel)
	panel.up.label = upLabel
	panel.up.bkg = upLabel.bkg
	upLabel:SetBlendWithParent(true)
	upLabel.bkg:SetBlendWithParent(true)
	
	local down = UIPushButton:Create(
		{0,0,arrowSize,arrowSize},
		{
			enabled=self.gfx.RightArrow,
			pressed=self.gfx.RightArrowPressed
		},
		{ pressed = UI.sfx.Command2 },
		{pressed=function(widget) ManipulatableObjectUI:ArrowPressed(panel, "down") end},
		nil,
		panel
	)
	down:RotateTo({arrowSize/2, arrowSize/2, 90}, {0,0,0})
	down:SetBlendWithParent(true)
	table.insert(panel.widgets, down)
	panel.down = down
	
	local downLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
	downLabel.bkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
	panel:AddChild(downLabel.bkg)
	panel:AddChild(downLabel)
	panel.down.label = downLabel
	panel.down.bkg = downLabel.bkg
	downLabel:SetBlendWithParent(true)
	downLabel.bkg:SetBlendWithParent(true)
	
	local keyName = PhysicalKeyName(key)
	
	UI:SetLabelText(label, keyName)
	UI:SizeLabelToContents(label)
	
	keyName = PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_ManipulateLeft])
	UI:SetLabelText(leftLabel, keyName)
	local llr, lld = UI:SizeLabelToContents(leftLabel)
	lld[3] = lld[3] - lld[1]
	lld[4] = lld[4] - lld[2]
	
	keyName = PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_ManipulateRight])
	UI:SetLabelText(rightLabel, keyName)
	local rlr, rld = UI:SizeLabelToContents(rightLabel)
	rld[3] = rld[3] - rld[1]
	rld[4] = rld[4] - rld[2]
	
	keyName = PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_ManipulateUp])
	UI:SetLabelText(upLabel, keyName)
	local ulr, uld = UI:SizeLabelToContents(upLabel)
	uld[3] = uld[3] - uld[1]
	uld[4] = uld[4] - uld[2]
	
	keyName = PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_ManipulateDown])
	UI:SetLabelText(downLabel, keyName)
	local dlr, dld = UI:SizeLabelToContents(downLabel)
	dld[3] = dld[3] - dld[1]
	dld[4] = dld[4] - dld[2]
	
	-- estimate panel size
	local gap = 8*UI.identityScale[1]
	local labelPad = 8*UI.identityScale[1]
	
	local d = label:Dimensions()
	local labelSize = {d[3]-d[1], d[4]-d[2]}
	
	local panelRect = {
		0,
		0,
		(arrowSize+gap)*2+labelSize[1]+(labelPad*8)+(lld[3]+rld[3]),
		(arrowSize+gap)*2+labelSize[2]+(labelPad*8)+(uld[3]+dld[3])
	}
	
	panel:SetRect(panelRect)
	
	local circleRect = {0, 0, labelPad*2+labelSize[1], labelPad*2+labelSize[2]}
	circle:SetRect(circleRect)
	circleRect = UI:CenterWidget(circle, panelRect)
	
	UI:CenterLabel(label, panelRect)
	
	local r = {0,0,0,0}
	
	r[3] = lld[3] + (labelPad*2)
	r[4] = lld[4] + (labelPad*2)
	leftLabel.bkg:SetRect(r)
	r = UI:VCenterWidget(leftLabel.bkg, panelRect, circleRect[1]-(gap*2)-arrowSize-r[3])
	UI:CenterLabel(leftLabel, r)
	
	r[3] = rld[3] + (labelPad*2)
	r[4] = rld[4] + (labelPad*2)
	rightLabel.bkg:SetRect(r)
	r = UI:VCenterWidget(rightLabel.bkg, panelRect, circleRect[1]+circleRect[3]+(gap*2)+arrowSize)
	UI:CenterLabel(rightLabel, r)
	
	r[3] = uld[3] + (labelPad*2)
	r[4] = uld[4] + (labelPad*2)
	upLabel.bkg:SetRect(r)
	r = UI:HCenterWidget(upLabel.bkg, panelRect, circleRect[2]-(gap*2)-arrowSize-r[4])
	UI:CenterLabel(upLabel, r)
	
	r[3] = dld[3] + (labelPad*2)
	r[4] = dld[4] + (labelPad*2)
	downLabel.bkg:SetRect(r)
	r = UI:HCenterWidget(downLabel.bkg, panelRect, circleRect[2]+circleRect[4]+(gap*2)+arrowSize)
	UI:CenterLabel(downLabel, r)
	
	UI:VCenterWidget(left, panelRect, circleRect[1]-gap-arrowSize)
	UI:VCenterWidget(right, panelRect, circleRect[1]+circleRect[3]+gap)
	UI:HCenterWidget(up, panelRect, circleRect[2]-gap-arrowSize)
	UI:HCenterWidget(down, panelRect, circleRect[2]+circleRect[4]+gap)
	
	panel:SetVisible(false)
	return panel
		
end

function ManipulatableObjectUI.UnmapPanel(self, panel)
	for k,v in pairs(panel.widgets) do
		v:Unmap()
	end
	panel:Unmap()
end

function ManipulatableObjectUI.ArrowPressed(self, panel, dir)
	panel.target:DoManipulateCommand(dir)
end

