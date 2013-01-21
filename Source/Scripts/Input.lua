-- Input.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function MapInputEvent(e)

	if (e.type == kI_KeyDown) or (e.type == kI_KeyUp) then
		return e
	end

	e.orig = { data={e.data[1], e.data[2], e.data[3]} }
	e.data[1] = e.data[1]
	e.data[2] = e.data[2]
	
	return e

end

function MapInputGesture(g)

	local vp = World.Viewport()
		
	g.orig = { mins={g.mins[1],g.mins[2]}, maxs={g.maxs[1],g.maxs[2]}, origin={g.origin[1], g.origin[2]} }
	
	g.mins[1] = g.mins[1]
	g.maxs[1] = g.maxs[1]
	g.origin[1] = g.origin[1]
	
	g.mins[2] = g.mins[2]
	g.maxs[2] = g.maxs[2]
	g.origin[2] = g.origin[2]

	return g

end

function World.OnInputEvent(e)

	if Input.IsTouchEvent(e) or (e.type == kI_MouseDown or e.type == kI_MouseUp) or (e.type == kI_KeyDown) then
		e = MapInputEvent(e)
		if (PlayerInput:OnInputEvent(e)) then
			return true
		end
	end
	
	return false

end

function World.OnInputGesture(g)

	if (PlayerInput:OnInputGesture(e)) then
		return true
	end
	
	return false

end

Input = {}
function Input.IsTouchBegin(e)
	return (e.type == kI_TouchBegin)
end

function Input.IsTouchEnd(e, touch)
	return ((touch == nil) or (e.touch == touch)) and 
		((e.type == kI_TouchEnd) or (e.type == kI_TouchCancelled))
end

function Input.IsTouchMove(e, touch)
	return  (e.touch == touch) and (e.type == kI_TouchMoved)
end

function Input.IsTouchEvent(e)
	return (e.type == kI_TouchBegin) or (e.type == kI_TouchEnd) or (e.type == kI_TouchMoved) or (e.type == kI_TouchStationary) or (e.type == kI_TouchCancelled)	
end

