-- AlertPanel.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

AlertPanel = {}
AlertPanel.OKButton = 1
AlertPanel.YesButton = 1
AlertPanel.CancelButton = 2
AlertPanel.NoButton = 2

function AlertPanel.Create(self)
	self.gfx = {}
	self.gfx.LineBorder = World.Load("UI/alertpanelbackground_M")
	self.sfx = {}
	self.sfx.Command = World.Load("Audio/ui_command")
	self.typefaces = {}
	self.typefaces.Text = World.Load("UI/AlertText_TF")
	self.typefaces.Title = World.Load("UI/AlertTitle_TF")
	
	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_AlertPanel, AlertPanel.EatInput)
	
	local panelWidth = 700*UI.identityScale[1]
	if (panelWidth < 700) then
		panelWidth = 700
	end
	
	self.widgets.panel = UI:CreateWidget("MatWidget", {rect={0, 0, panelWidth, panelWidth*9/16}, material=self.gfx.LineBorder})
	self.widgets.panel:SetHAlign(kHorizontalAlign_Center)
	self.widgets.panel:SetVAlign(kVerticalAlign_Center)
	self.widgets.root:AddChild(self.widgets.panel)
	
	self.widgets.title = UI:CreateWidget("TextLabel", {rect={0,16*UI.identityScale[2],8,8}, typeface=self.typefaces.Title})
	self.widgets.panel:AddChild(self.widgets.title)
	self.widgets.text = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.Text})
	self.widgets.panel:AddChild(self.widgets.text)
		
	-- scale buttons accordingly:
	local buttonSize = UI.gfx.Button:Dimensions()
	
	buttonSize[1] = buttonSize[1] * 0.8
	buttonSize[2] = buttonSize[2] * 0.8
	
	local panelRect = self.widgets.panel:Rect()
	local titleSize = UI:FontAdvanceSize(self.typefaces.Title)+(48*UI.identityScale[2])
	local textRect = {
		48*UI.identityScale[1],
		titleSize,
		panelRect[3] - (48*2*UI.identityScale[1]),
		panelRect[4] - titleSize - (48*UI.identityScale[2]) - buttonSize[2]
	}
	
	self.widgets.text:SetRect(textRect)
	
	self.widgets.buttonAbk = UI:CreateWidget("MatWidget", {rect={0,0,buttonSize[1], buttonSize[2]}, material=UI.gfx.Solid})
	self.widgets.buttonAbk:BlendTo({0,0,0,1}, 0)
	self.widgets.panel:AddChild(self.widgets.buttonAbk)
	
	self.widgets.buttonBbk = UI:CreateWidget("MatWidget", {rect={0,0,buttonSize[1], buttonSize[2]}, material=UI.gfx.Solid})
	self.widgets.buttonBbk:BlendTo({0,0,0,1}, 0)
	self.widgets.panel:AddChild(self.widgets.buttonBbk)
	
	self.widgets.buttonA = UI:CreateStylePushButton({0,0,buttonSize[1],buttonSize[2]}, AlertPanel.ButtonPressed, {fontSize="small"}, self.widgets.panel)
	self.widgets.buttonB = UI:CreateStylePushButton({0,0,buttonSize[1],buttonSize[2]}, AlertPanel.ButtonPressed, {fontSize="small"}, self.widgets.panel)
	
	self.widgets.root:SetVisible(false)
	
	self.active = false
end

function AlertPanel.EatInput(widget, e)
	return true
end

function AlertPanel.Dismiss(self, result)

	self.active = false
	self.widgets.title:BlendTo({1,1,1,0}, 0.1)
	self.widgets.text:BlendTo({1,1,1,0}, 0.1)
	self.widgets.buttonA:BlendTo({1,1,1,0}, 0.1)
	self.widgets.buttonAbk:BlendTo({0,0,0,0}, 0.1)
	self.widgets.buttonB:BlendTo({1,1,1,0}, 0.1)
	self.widgets.buttonBbk:BlendTo({0,0,0,0}, 0.1)
	self.widgets.buttonA.class:SetEnabled(self.widgets.buttonA, false)
	self.widgets.buttonB.class:SetEnabled(self.widgets.buttonB, false)
	
	local f = function ()
		self.widgets.panel:ScaleTo({0,0}, {0.3, 0.3})
		local f = function()
			self.widgets.root:SetVisible(false)
			local callback = self.callback
			self.callback = nil
			if (callback) then
				callback(result)
			end
		end
		World.globalTimers:Add(f, 0.3)
	end
	
	World.globalTimers:Add(f, 0.1)
end

function AlertPanel.ButtonPressed(widget, e)
	AlertPanel:Dismiss(widget.code)
end

function AlertPanel.Run(self, title, msg, buttons, callback, screenRect)
	
	if (screenRect == nil) then
		screenRect = UI.fullscreenRect
	end
	
	UI:CenterWidget(self.widgets.panel, screenRect)
	
	assert(self.active == false)
	
	self.callback = callback
	
	-- inset the text a little
	local buttonRect = self.widgets.buttonA:Rect()
	local panelRect = self.widgets.panel:Rect()
	
	local text = StringTable.Get(title)
	UI:SetLabelText(self.widgets.title, text)
	UI:HCenterLabel(self.widgets.title, {0, 0, panelRect[3], panelRect[4]})
			
	text = StringTable.Get(msg)
	UI:LineWrapLJustifyText(
		self.widgets.text,
		nil,
		true,
		6,
		text,
		UI.fontScale,
		UI.invFontScale
	)
	
	text = StringTable.Get(buttons[1][1])
	UI:LineWrapCenterText(
		self.widgets.buttonA.label,
		nil,
		nil,
		2,
		text,
		UI.fontScale,
		UI.invFontScale
	)
	
	self.widgets.buttonA.code = buttons[1].r
	
	local yButtonPos = panelRect[4] - buttonRect[4] - (8*UI.identityScale[2])
	local doButtonB = false
	
	if (buttons[2]) then -- second button?
		text = StringTable.Get(buttons[2][1])
		UI:LineWrapCenterText(
			self.widgets.buttonB.label,
			nil,
			nil,
			2,
			text,
			UI.fontScale,
			UI.invFontScale
		)
		
		local xInset = 64*UI.identityScale[1]
		UI:MoveWidget(self.widgets.buttonA, xInset, yButtonPos)
		UI:MoveWidget(self.widgets.buttonAbk, xInset, yButtonPos)
		UI:MoveWidget(self.widgets.buttonB, panelRect[3]-xInset-buttonRect[3], yButtonPos)
		UI:MoveWidget(self.widgets.buttonBbk, panelRect[3]-xInset-buttonRect[3], yButtonPos)
		self.widgets.buttonB.code = buttons[2].r
		doButtonB = true
	else
		UI:MoveWidget(self.widgets.buttonA, 0, yButtonPos)
		UI:HCenterWidget(self.widgets.buttonA, {0,0,panelRect[3],panelRect[4]})
		local r = self.widgets.buttonA:Rect()
		self.widgets.buttonAbk:SetRect(r)
	end
	
	self.widgets.buttonA.class:SetEnabled(self.widgets.buttonA, true)
	self.widgets.buttonB.class:SetEnabled(self.widgets.buttonB, true)
	
	self.widgets.title:BlendTo({1,1,1,0},0)
	self.widgets.text:BlendTo({1,1,1,0},0)
	self.widgets.buttonA:BlendTo({1,1,1,0},0)
	self.widgets.buttonAbk:BlendTo({0,0,0,0},0)
	self.widgets.buttonB:BlendTo({1,1,1,0},0)
	self.widgets.buttonBbk:BlendTo({0,0,0,0},0)
	self.widgets.panel:ScaleTo({0,0}, {0,0})
	
	local f = function()
		self.widgets.title:BlendTo({1,1,1,1}, 0.3)
		self.widgets.text:BlendTo({1,1,1,1}, 0.3)
		self.widgets.buttonA:BlendTo({1,1,1,1}, 0.3)
		self.widgets.buttonAbk:BlendTo({0,0,0,1},0.3)
		if (doButtonB) then
			self.widgets.buttonB:BlendTo({1,1,1,1}, 0.3)
			self.widgets.buttonBbk:BlendTo({0,0,0,1},0.3)
		end
	end
	
	self.widgets.root:SetVisible(true)
	self.widgets.panel:ScaleTo({1,1}, {0.3, 0.3})
	World.globalTimers:Add(f, 0.3)

end

function AlertPanel.OK(self, title, msg, callback, screenRect)
	AlertPanel:Run(title, msg, {{"ALERT_PANEL_OK", r=AlertPanel.OKButton}}, callback, screenRect)
end

function AlertPanel.OKCancel(self, title, msg, screenRect)
	AlertPanel:Run(
		title,
		msg, 
		{
			{"ALERT_PANEL_OK", r=AlertPanel.OKButton}, 
			{"ALERT_PANEL_CANCEL", r=AlertPanel.CancelButton}
		}, 
		callback,
		screenRect
	)
end

function AlertPanel.YesNo(self, title, msg, callback, screenRect)
	AlertPanel:Run(
		title,
		msg, 
		{
			{"ALERT_PANEL_YES", r=AlertPanel.YesButton}, 
			{"ALERT_PANEL_NO", r=AlertPanel.NoButton}
		}, 
		callback,
		screenRect
	)
end
