-- UIWidgets.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

UIWidget = Class:New()

--[[---------------------------------------------------------------------------
	UIPushButton
-----------------------------------------------------------------------------]]

UIPushButton = UIWidget:New()

--[[---------------------------------------------------------------------------
	Creates a push button widget:
	
	size = {x, y, w, h}
	gfx = {
		pressed,
		unpressed
	}
	sfx = {
		pressed,
		unpressed,
		disabled
	}
	events = {
		filter,
		pressed,
		unpressed
	}
	options = {
		manualUnpress
			-- Call widget.class:Reset(widget) to unpressed widget (not done when user
			lifts finger)
	}
-----------------------------------------------------------------------------]]
function UIPushButton.Create(self, size, gfx, sfx, events, options, layer)

	local params = {
		rect = size,
		material = gfx.pressed,
		OnInputEvent = UIPushButton.OnInputEvent
	}
	
	local widget = UI:CreateWidget("MatWidget", params)
	widget.events = events or {}
	widget.sfx = sfx or {}
	widget.gfx = gfx or {}
	widget.options = options or {}
	widget.class = UIPushButton
	widget.state = { pressed = false }
	widget.enabled = true
	
	if (layer) then
		layer:AddChild(widget)
	end
	
	return widget
end

function UIPushButton.OnInputEvent(widget, e)
	
	if (widget.events.filter) then
		x, r = widget.events.filter(widget, e)
		if (x) then
			return r
		end
	end
	
	if (Input.IsTouchBegin(e)) then
		return UIPushButton:DoPressed(widget, e)
	elseif ((widget.state.pressed) and Input.IsTouchMove(e, widget.busy)) then
		return true
	elseif ((widget.state.pressed) and widget.busy and Input.IsTouchEnd(e, widget.busy)) then
		if (widget.options.manualUnpress) then
			widget.busy = nil
			return true
		end
		return UIPushButton:DoUnpressed(widget)
	end

	return false

end

function UIPushButton.DoPressed(self, widget, e)
	if (widget.state.pressed) then
		return true -- eat this press
	end
	
	if (UI.gestureMode) then
		return false -- gesturing, all UI is passthrough
	end
	
	if (not widget.enabled) then
		if (widget.sfx.disabled) then
			widget.sfx.disabled:Play(kSoundChannel_UI, 0)
		end
		return true
	end
	
	widget.busy = e
	widget.state.pressed = true
	
	if (widget.gfx.pressed) then
		widget:SetMaterial(widget.gfx.pressed)
	end
	
	if (widget.sfx.pressed) then
		widget.sfx.pressed:Play(kSoundChannel_UI, 0)
	end
	
	if (widget.events.pressed) then
		widget.events.pressed(widget, e)
	end
	
end

function UIPushButton.DoUnpressed(self, widget)

	widget.busy = nil
	
	if (widget.gfx.unpressed) then
		widget:SetMaterial(widget.gfx.unpressed)
	end
	
	if (widget.sfx.unpressed) then
		widget.sfx.unpressed:Play(kSoundChannel_UI, 0)
	end
	
	if (widget.events.unpressed) then
		widget.events.unpressed(widget)
	end
	
end

function UIPushButton.Reset(self, widget)
	self:DoUnpressed(widget)
end
