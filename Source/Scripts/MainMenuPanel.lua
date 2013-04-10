-- MainMenuPanel.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenuPanel = Class:New()

function MainMenuPanel.Create(self, options, parent)

	if (options == nil) then
		options = {}
	end
	
	local rect = options.rect
	if (rect == nil) then
		rect = {0,0,8,8}
		options.rect = rect
	end
	
	local OnInputEvent = function(widget, e)
		return self:OnInputEvent(e)
	end

	self.widgets = {}
	self.widgets.panel = UI:CreateWidget("MatWidget", {material=MainMenu.gfx.MMPanel, rect=rect, OnInputEvent=OnInputEvent})
	
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

function MainMenuPanel.TransitionIn(self, scale, time, fade, onComplete)

	self.eatInput = true
	
	self:PrepareContents()
	self.widgets.panel:ScaleTo({0,0}, {0,0})
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

function MainMenuPanel.TransitionOut(self, time, fade, onComplete)

	self.eatInput = true
	
	self:FadeContents({1,1,1,0}, fade)
	
	local f = function()
		local f = function()
			if (onComplete) then
				onComplete()
			end
		end
		self.widgets.panel:ScaleTo({0,0}, {time,time})
		World.globalTimers:Add(f, time, true)
	end
	
	World.globalTimers:Add(f, fade, true)
end

function MainMenuPanel.PrepareContents(self)
	MainMenuPanel.FadeContents(self, {1,1,1,0}, 0)
end

function MainMenuPanel.AnimateContents(self, onComplete)
	if (onComplete) then
		onComplete()
	end
end

function MainMenuPanel.FadeContents(self, color, time)

end
