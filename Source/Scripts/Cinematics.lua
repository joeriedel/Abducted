-- Cinematics.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Cinematics = Class:New()
Cinematics.busy = 0
Cinematics.Active = LL_New()
Cinematics.Persistent = LL_New()

function Cinematics.Play(self, args, time, originEntity)
		
	x = Tokenize(args)
	local animateCamera = not FindArrayElement(x, "camera=no")
	local looping = FindArrayElement(x, "loop=true")
	local playForever = FindArrayElement(x, "forever=true")
	local interactive = FindArrayElement(x, "interactive=true")
	local flags = 0
	
	if (animateCamera) then
		flags = bit.bor(flags, kCinematicFlag_AnimateCamera)
	end
	
	if (looping and playForever) then
		COutLine(kC_Debug, "WARNING: playing a cinematic with loop=true and forever=true makes no sense, ignoring.")
		return
	end
	
	if (playForever) then
		flags = bit.bor(flags, kCinematicFlag_CanPlayForever)
	end
	
	if (looping) then
		flags = bit.bor(flags, kCinematicFlag_Loop)
	end
	
	local item = LL_Append(Cinematics.Active, {cmd=args, name=x[1]})
	
	if (forever) then
		item.persistent = LL_Append(Cinematics.Persistent, {cmd=args, name=x[1]})
	end
	
	if (interactive) then
		local callbacks = {
			OnTag = function(self, tag)
				World.PostEvent(tag)
			end,
			OnComplete = function(self)
				LL_Remove(Cinematics.Active, item)
				if (item.persistent) then
					LL_Remove(Cinematics.Persistent, item.persistent)
				end
			end
		}
		if (not World.PlayCinematic(x[1], flags, 0, originEntity, Game.entity, callbacks)) then
			LL_Remove(Cinematics.Active, item)
			if (item.persistent) then
				LL_Remove(Cinematics.Persistent, item.persistent)
			end
			time = nil
		end
	else
		local callbacks = {
			OnTag = function(self, tag)
				World.PostEvent(tag)
			end,
			OnComplete = function(self)
				LL_Remove(Cinematics.Active, item)
				if (item.persistent) then
					LL_Remove(Cinematics.Persistent, item.persistent)
				end
				Cinematics.busy = Cinematics.busy - 1
				if (Cinematics.busy == 0) then
					HUD:SetVisible(true)
				end
			end
		}
		
		if (World.PlayCinematic(x[1], flags, 0, originEntity, Game.entity, callbacks)) then
			if (self.busy == 0) then
				HUD:SetVisible(false)
			end
			self.busy = self.busy + 1
		else
			LL_Remove(Cinematics.Active, item)
			if (item.persistent) then
				LL_Remove(Cinematics.Persistent, item.persistent)
			end
			time = nil
		end
	end
	
	if (time) then
		World.SetCinematicTime(x[1], time)
	end
end

function Cinematics.Stop(self, name)

	item = LL_Head(Cinematics.Active)
	
	while (item) do
	
		if (item.name == name) then
			item = LL_Remove(Cinematics.Active, item)
			if (item.persistent) then
				LL_Remove(Cinematics.Persistent, item.persistent)
			end
			break
		else
			item = LL_Next(item)
		end
	
	end
	
	World.StopCinematic(name)

end

function Cinematics.PlayLevelCinematics(self)
	World.PlayCinematic("environment", kCinematicFlag_CanPlayForever, 0)
	World.PlayCinematic("environment_loop", kCinematicFlag_Loop, 0)
	if (not GameDB.loadingCheckpoint) then
		Cinematics:Play("intro")
	end
end

function Cinematics.SaveState(self)

	local cmds = {}
	local times = {}
	
	local f = function(x)
		table.insert(cmds, x)
		local t = World.CinematicTime(x.name)
		table.insert(times, t)
	end
	
	LL_Do(Cinematics.Persistent, f)
	
	if (next(cmds) == nil) then
	
		Persistence.DeleteKey(SaveGame, "activeCinematics")
		Persistence.DeleteKey(SaveGame, "activeCinematicTimes")
	
	else
	
		local cs = nil
		local ts = nil
		
		for k,v in pairs(cmds) do
		
			if (cs == nil) then
				cs = v.cmd
				ts = tostring(times[k])
			else
				cs = cs..";"..v.cmd
				ts = ts..";"..tostring(times[k])
			end
		
		end
		
		Persistence.WriteString(SaveGame, "activeCinematics", cs)
		Persistence.WriteString(SaveGame, "activeCinematicTimes", ts)
	
	end

end

function Cinematics.LoadState(self)

	-- stop active cinematics
	local f = function(x)
		World.StopCinematic(x.name, true)
	end
	
	LL_Do(Cinematics.Active, f)
	Cinematics.Active = LL_New()
	Cinematics.Persistent = LL_New()
	
	self.busy = 0
	
	local cmds = Persistence.ReadString(SaveGame, "activeCinematics")
	if (cmds) then
		cmds = string.split(cmds, ";")
		local times = Persistence.ReadString(SaveGame, "activeCinematicTimes")
		assert(times)
		times = string.split(times, ";")
		
		for k,v in pairs(cmds) do
			Cinematics:Play(v, tonumber(times[k]))
		end
	end

end

