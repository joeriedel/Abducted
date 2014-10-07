-- MainMenuInstructions.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms


MainMenu.Instructions = MainMenuPanel:New()

function MainMenu.InitInstructions(self)

	self.instructionsPanel = MainMenu.Instructions:New()
	self.instructionsPanel:Create({rect=self.contentRect}, self.widgets.root)
	self.instructionsPanel:Layout()
	self.instructionsPanel.widgets.panel:SetHAlign(kHorizontalAlign_Left)
	self.instructionsPanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	self.instructionsPanel.widgets.panel:SetVisible(false)
	
end

function MainMenu.Instructions.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
    local panelRect = self.widgets.panel:Rect()
    local panelArea = {
	    0,
		0,
		panelRect[3],
		panelRect[4]
		}
		        
	self.widgets.vlist = UI:CreateWidget("VListWidget", {rect=panelArea})

	if (UI.mode == kGameUIMode_PC) then
        UI:CreateVListWidgetScrollBar(
            self.widgets.vlist,
	        24,
	        24,
	        8
            )
		panelArea[3] = panelArea[3] - 24
    end
	
	self.widgets.vlist:SetClipRect(panelArea)
    self.widgets.vlist:SetEndStops({0, panelRect[4]*0.1})
	self.widgets.vlist:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.vlist)

end

function MainMenu.Instructions.Layout(self)

	MainMenuPanel.Layout(self)
	
	local panelRect = self.widgets.panel:Rect()
        
	if (UI.mode == kGameUIMode_PC) then
		panelRect[3] = panelRect[3] - 24 -- make room for scroll bar
	end
        
	self.widgets.vlist:Clear()
	
	local image = World.Load("UI/instructions_M")
	local d = image:Dimensions()
	
	local w = panelRect[3]
	local h = (w/d[1])*d[2]
	
	local widget = UI:CreateWidget("MatWidget", {rect={0,0,w,h}, material=image})
	widget:SetBlendWithParent(true)
	self.widgets.vlist:AddItem(widget)
				
	self.widgets.vlist:RecalcLayout()
	self.widgets.vlist:ScrollTo({0,0}, 0)
	
end

function MainMenu.Instructions.PrepareContents(self)
	MainMenuPanel.PrepareContents(self)
	self.widgets.vlist:BlendTo({1,1,1,0}, 0)
	self.widgets.vlist:ScrollTo({0,0}, 0)
	MainMenu.widgets.logo:BlendTo({0,0,0,1}, 0.5)
end

function MainMenu.Instructions.AnimateContents(self, onComplete)
	self.widgets.vlist:BlendTo({1,1,1,1}, 0.2)
	if (onComplete) then
		local f = function()
			onComplete()
		end
		World.globalTimers:Add(f, 0.2)
	end
end

function MainMenu.Instructions.FadeOutContents(self, time)
	self.widgets.vlist:BlendTo({1,1,1,0}, time)
	MainMenu.widgets.logo:BlendTo({1,1,1,1}, 0.5)
end
