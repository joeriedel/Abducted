-- AlertPanel.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

AlertPanel = {}
AlertPanel.OKButton = 1
AlertPanel.CancelButton = 2

function AlertPanel.Create(self)
	self.gfx = {}
	self.gfx.LineBorder = World.Load("UI/alertpanelbackground_M")
	self.sfx = {}
	self.sfx.Command = World.Load("Audio/ui_command")
	self.typefaces = {}
	self.typefaces.Default = World.Load("UI/ChatChoice_TF")
	
	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_AlertPanel, AlertPanel.EatInput)
	
	self.widgets.panel = UI:CreateWidget("MatWidget", {rect={0, 0, UI.screenWidth*0.5, UI.screenHeight*0.5}, material=self.gfx.LineBorder})
	self.widgets.panel:SetHAlign(kHorizontalAlign_Center)
	self.widgets.panel:SetVAlign(kVerticalAlign_Center)
	self.widgets.root:AddChild(self.widgets.panel)
	
	self.widgets.text = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.Default})
	self.widgets.panel:AddChild(self.widgets.text)
	UI:CenterWidget(self.widgets.panel, UI.fullscreenRect)
	
	-- scale buttons accordingly:
	local buttonSize = UI.gfx.Button:Dimensions()
	
	buttonSize[1] = buttonSize[1] * 0.7
	buttonSize[2] = buttonSize[2] * 0.7
	
	local panelRect = self.widgets.panel:Rect()
	local textRect = {
		16*UI.identityScale[1],
		16*UI.identityScale[2],
		panelRect[3] - (32*UI.identityScale[1]),
		panelRect[4] - (32*UI.identityScale[2]) - buttonSize[2]
	}
	
	self.widgets.text:SetRect(textRect)
	
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
	self.widgets.text:BlendTo({1,1,1,0}, 0.1)
	self.widgets.buttonA:BlendTo({1,1,1,0}, 0.1)
	self.widgets.buttonB:BlendTo({1,1,1,0}, 0.1)
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
		World.globalTimers:Add(f, 0.3, true)
	end
	
	World.globalTimers:Add(f, 0.1, true)
end

function AlertPanel.ButtonPressed(widget, e)
	AlertPanel:Dismiss(widget.code)
end

function AlertPanel.Run(self, msg, buttons, callback)
	
	assert(self.active == false)
	
	self.callback = callback
	
	-- inset the text a little
	local buttonRect = self.widgets.buttonA:Rect()
	local panelRect = self.widgets.panel:Rect()
	
	
	local text = StringTable.Get(msg)
	UI:LineWrapLJustifyText(
		self.widgets.text,
		nil,
		true,
		0,
		text,
		UI.fontScale,
		UI.invFontScale
	)
	
	text = StringTable.Get(buttons[1][1])
	UI:LineWrapCenterText(
		self.widgets.buttonA.label,
		nil,
		nil,
		0,
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
			0,
			text,
			UI.fontScale,
			UI.invFontScale
		)
		
		local xInset = 64*UI.identityScale[1]
		UI:MoveWidget(self.widgets.buttonA, xInset, yButtonPos)
		UI:MoveWidget(self.widgets.buttonB, panelRect[3]-xInset-buttonRect[3], yButtonPos)
		self.widgets.buttonB.code = buttons[2].r
		doButtonB = true
	else
		UI:MoveWidget(self.widgets.buttonA, 0, yButtonPos)
		UI:CenterWidget(self.widgets.buttonA, {0,0,panelRect[3],panelRect[4]})
	end
	
	self.widgets.buttonA.class:SetEnabled(self.widgets.buttonA, true)
	self.widgets.buttonB.class:SetEnabled(self.widgets.buttonB, true)
	
	self.widgets.text:BlendTo({1,1,1,0},0)
	self.widgets.buttonA:BlendTo({1,1,1,0},0)
	self.widgets.buttonB:BlendTo({1,1,1,0},0)
	self.widgets.panel:ScaleTo({0,0}, {0,0})
	
	local f = function()
		self.widgets.text:BlendTo({1,1,1,1}, 0.3)
		self.widgets.buttonA:BlendTo({1,1,1,1}, 0.3)
		if (doButtonB) then
			self.widgets.buttonB:BlendTo({1,1,1,1}, 0.3)
		end
	end
	
	self.widgets.root:SetVisible(true)
	self.widgets.panel:ScaleTo({1,1}, {0.3, 0.3})
	World.globalTimers:Add(f, 0.3, true)

end

function AlertPanel.OK(self, msg, callback)
	AlertPanel:Run(msg, {{"ALERT_PANEL_OK", r=AlertPanel.OKButton}}, callback)
end

function AlertPanel.OKCancel(self, msg)
	AlertPanel:Run(
		msg, 
		{
			{"ALERT_PANEL_OK", r=AlertPanel.OKButton}, 
			{"ALERT_PANEL_CANCEL", r=AlertPanel.CancelButton}
		}, 
		callback
	)
end
