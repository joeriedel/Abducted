-- Input.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function MapInputEvent(e)

	if (e.type == I_KeyDown) or (e.type == I_KeyUp) then
		return e
	end

	local vp = World.Viewport()
	local sw = 1024/vp[1]
	local sh = 768/vp[2]
	
	e.orig = { data={e.data[1], e.data[2], e.data[3]} }
	e.data[1] = e.data[1]*sw
	e.data[2] = e.data[2]*sh
	
	return e

end

function MapInputGesture(g)

	local vp = World.Viewport()
	local sw = 1024/vp[1]
	local sh = 768/vp[2]
	
	g.orig = { mins={g.mins[1],g.mins[2]}, maxs={g.maxs[1],g.maxs[2]}, origin={g.origin[1], g.origin[2]} }
	
	g.mins[1] = g.mins[1]*sw
	g.maxs[1] = g.maxs[1]*sw
	g.origin[1] = g.origin[1]*sw
	
	g.mins[2] = g.mins[2]*sh
	g.maxs[2] = g.maxs[2]*sh
	g.origin[2] = g.origin[2]*sh

	return g

end

function World.OnInputEvent(e)

	if Input.IsTouchEvent(e) or (e.type == I_MouseDown or e.type == I_MouseUp) or (e.type == I_KeyDown) then
		e = MapInputEvent(e)
		if World.game and World.game.OnInputEvent then
			return World.game:OnInputEvent(e)
		end
	
	end
	
	return false

end

function World.OnInputGesture(g)

	if World.game and World.game.OnInputGesture then
		g = MapInputGesture(g)
		return World.game:OnInputGesture(g)
	end
	
	return false

end

Input = {}
function Input.IsTouchBegin(e)
	return (e.type == I_TouchBegin)
end

function Input.IsTouchEnd(e, touch)
	return (e.touch == touch) and 
		((e.type == I_TouchEnd) or (e.type == I_TouchCancelled))
end

function Input.IsTouchEndAny(e)
	return ((e.type == I_TouchEnd) or (e.type == I_TouchCancelled))
end

function Input.IsTouchMove(e, touch)
	return  (e.touch == touch) and (e.type == I_TouchMoved)
end

function Input.IsTouchEvent(e)
	return (e.type == I_TouchBegin) or (e.type == I_TouchEnd) or (e.type == I_TouchMoved) or (e.type == I_TouchStationary) or (e.type == I_TouchCancelled)	
end

