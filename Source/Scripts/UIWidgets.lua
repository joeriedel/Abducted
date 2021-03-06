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
		highlight
			-- button has highlight pressed state
		label
			-- button has a label
	}
-----------------------------------------------------------------------------]]
function UIPushButton.Create(self, size, gfx, sfx, events, options, parent)

	gfx.pressed = gfx.pressed or gfx.enabled
	gfx.disabled = gfx.disabled or gfx.enabled
	
	local params = {
		rect = size,
		material = gfx.enabled,
		OnInputEvent = UIPushButton.OnInputEvent
	}
	
	local widget = UI:CreateWidget("MatWidget", params)
	widget.events = events or {}
	widget.sfx = sfx or {}
	widget.gfx = gfx or {}
	widget.options = options or {}
	widget.class = UIPushButton
	widget.state = { pressed = false }
	
	if (widget.options.enabled ~= nil) then
		widget.enabled = widget.options.enabled
	else
		widget.enabled = true
	end
	
	if (widget.options.highlight) then
		local w = UI:CreateWidget("MatWidget", {rect={0,0,params.rect[3], params.rect[4]}, material=widget.gfx.highlight})
		w:BlendTo(widget.options.highlight.off, 0)
		widget:AddChild(w)
		widget.highlight = w
		w:SetBlendWithParent(true)
	end
	
	if (widget.options.label) then
		local w = UI:CreateWidget("TextLabel", {rect={0,0,params.rect[3], params.rect[4]}, typeface=widget.options.label.typeface})
		widget:AddChild(w)
		widget.label = w
		w:SetBlendWithParent(true)
	end
	
	if (parent) then
		parent:AddChild(widget)
	end
	
	if (not widget.enabled) then
		UIPushButton:SetGfxState(widget, true)
	end
	
	return widget
end

function UIPushButton.SetEnabled(self, widget, enabled)
	if (widget.enabled == enabled) then
		return
	end
	
	widget.enabled = enabled
	widget.state.pressed = false
	widget.busy = nil
	widget:SetCapture(false)
	self:SetGfxState(widget, true)
end

function UIPushButton.OnInputEvent(widget, e)
	
	if (widget.events.filter) then
		x, r = widget.events.filter(widget, e)
		if (x) then
			return r
		end
	end
	
	if (Input.IsTouchBegin(e)) then
		local handled, pressed = UIPushButton:DoPressed(widget, e)
		if (handled and pressed) then
			if (widget.options.pressedWhenDown) then
				if (widget.sfx.pressed) then
					widget.sfx.pressed:Play(kSoundChannel_UI, 0)
				end
				if (widget.events.pressed) then
					widget.events.pressed(widget, e)
				end
			end
		end
		return handled
	elseif ((widget.state.pressed) and Input.IsTouchMove(e)) then
		return true
	elseif ((widget.state.pressed) and widget.busy and Input.IsTouchEnd(e, widget.busy)) then
		if (e.type == kI_TouchCancelled) then
			widget.busy = nil
			widget.state.pressed = false
			UIPushButton:SetGfxState(widget)
			widget:SetCapture(false)
		else
			if (widget.options.manualUnpress) then
				widget.busy = nil
				widget:SetCapture(false)
				return true
			end
			if (UIPushButton:DoUnpressed(widget, true) and (not widget.options.pressedWhenDown)) then
				if (widget.sfx.pressed) then
					widget.sfx.pressed:Play(kSoundChannel_UI, 0)
				end
				if (widget.events.pressed) then
					widget.events.pressed(widget, e)
				end
			end
		end
		return true
	end

	return false

end

function UIPushButton.DoPressed(self, widget, e)
	if (widget.state.pressed) then
		return true, false -- eat this press
	end
	
	if (UI.gestureMode) then
		return false -- gesturing, all UI is passthrough
	end
	
	if (not widget.enabled) then
		if (widget.sfx.disabled) then
			widget.sfx.disabled:Play(kSoundChannel_UI, 0)
		end
		return true, false
	end
	
	if (widget.state.timer) then
		widget.state.timer:Clean()
		widget.state.timer = nil
	end
	
	widget.busy = e.touch
	widget.state.pressed = true
	widget.state.time = Game.sysTime
	
	widget.class:SetGfxState(widget)
	
	widget:SetCapture(true)
	
	if (widget.events.stateNotify) then
		widget.events.stateNotify(widget)
	end
	
	return true, true
	
end

function UIPushButton.DoUnpressed(self, widget, event)

	widget.busy = nil
	widget.state.pressed = false
	widget:SetCapture(false)
	
	if (widget.gfx.enabled) then
		widget:SetMaterial(widget.gfx.enabled)
	end
	
	if (widget.sfx.unpressed and event) then
		widget.sfx.unpressed:Play(kSoundChannel_UI, 0)
	end
	
	if (widget.events.stateNotify) then
		widget.events.stateNotify(widget)
	end
	
	if (widget.events.unpressed and event) then
		widget.events.unpressed(widget)
	end
	
	if (widget.highlight) then
		local options = widget.options
		if (options.highlight.overbright) then
			local delta = Game.sysTime - widget.state.time
			if (delta < options.highlight.overbrightTime) then
				-- no chance to see overbright state use a timer to readjust
				delta = options.highlight.overbrightTime - delta
				local f = function()
					widget.highlight:BlendTo(options.highlight.on, options.highlight.time)
				end
				widget.state.timer = World.globalTimers:Add(f, delta)
			else
				widget.highlight:BlendTo(options.highlight.on, options.highlight.time)
			end
		end
	end
	
	return true
	
end

function UIPushButton.Reset(self, widget)
	self:DoUnpressed(widget)
end

function UIPushButton.ResetHighlight(self, widget, immediate)
	if (widget.highlight) then
		if (immediate) then
			widget.highlight:BlendTo(widget.options.highlight.off, 0)
		else
			widget.highlight:BlendTo(widget.options.highlight.off, widget.options.highlight.time)
		end
	end
end

function UIPushButton.SetGfxState(self, widget, immediate)
	if (widget.state.timer) then
		widget.state.timer:Clean()
		widget.state.timer = nil
	end
	
	if (widget.disableGfxChanges) then
		return
	end
	
	if (widget.state.pressed) then
		if (widget.gfx.pressed) then
			widget:SetMaterial(widget.gfx.pressed)
		end
		if (widget.highlight) then
			if (immediate) then
				widget.highlight:BlendTo(options.highlight.on, 0)
			else
				local options = widget.options
				if (options.highlight.overbright) then
					widget.highlight:BlendTo(options.highlight.overbright, options.highlight.overbrightTime)
				else
					widget.highlight:BlendTo(options.highlight.on, options.highlight.time)
				end
			end
		end
	else
		if (widget.enabled) then
			if (widget.gfx.enabled) then
				widget:SetMaterial(widget.gfx.enabled)
			end
		else
			if (widget.gfx.disabled) then
				widget:SetMaterial(widget.gfx.disabled)
			end
		end
		
		if (widget.highlight) then
			widget.highlight:BlendTo(widget.options.highlight.off, 0)
		end
	end
end

function UIPushButton.ChangeGfx(self, widget, gfx)
	gfx.disabled = gfx.disabled or gfx.enabled
	gfx.pressed  = gfx.pressed or gfx.enabled
	
	widget.gfx = gfx
		
	self:SetGfxState(widget)
end
