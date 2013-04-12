-- MainMenuPanel.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenuPanel = Class:New()

function MainMenuPanel.Create(self, options, parent)

	options = options or {}
	
	if (options.rect == nil) then
		options.rect = {0,0,8,8}
	end
	
	if (options.background == nil) then
		options.background = MainMenu.gfx.MMPanel
	end
	
	local OnInputEvent = function(widget, e)
		return self:OnInputEvent(e)
	end

	self.widgets = {}
	self.widgets.panel = UI:CreateWidget("MatWidget", {material=options.background, rect=options.rect, OnInputEvent=OnInputEvent})
	
	if (parent) then
		parent:AddChild(self.widgets.panel)
	end
	
	return options
	
end

function MainMenuPanel.Show(self, show)
	self.widgets.panel:SetVisible(show)
end

function MainMenuPanel.OnInputEvent(self, e)

	if (self.eatInput) then
		return true
	end
	
	return false

end

function MainMenuPanel.Layout(self)

end

function MainMenuPanel.AddBlendedBorders(self)

end

function MainMenuPanel.TransitionIn(self, scale, time, onComplete)

	self.eatInput = true
	
	self:PrepareContents()
	self.widgets.panel:ScaleTo(scale, {0,0})
	self.widgets.panel:ScaleTo({1,1}, {time,time})
	self.widgets.panel:SetVisible(true)
	
	local f = function()
		local f = function()
			self.eatInput = false
			if (onComplete) then
				onComplete()
			end
		end
		self:AnimateContents(f)
	end
	
	World.globalTimers:Add(f, time, true)

end

function MainMenuPanel.TransitionOut(self, scale, time, fade, onComplete)

	self.eatInput = true
	
	self:FadeOutContents(fade)
	
	local f = function()
		self.widgets.panel:ScaleTo(scale, {time,time})
		if (onComplete) then
			local f = function()
				onComplete()
			end
			World.globalTimers:Add(f, time, true)
		end
	end
	
	World.globalTimers:Add(f, fade, true)
end

function MainMenuPanel.PrepareContents(self)

end

function MainMenuPanel.AnimateContents(self, onComplete)
	if (onComplete) then
		onComplete()
	end
end
