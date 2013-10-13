-- CVars.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
		CVar Support
		
		CVars are variables that can be set from the debug console, or inside
		game code, or engine code. They are fully exposed to lua.
		
		CVars support bool, int, float, string, and func types. Func types
		execute a function when invoked.
		
		Additionally, all cvars are exposed through the network via the
		debug console, meaning you can set/test.
		
		NOTE: All cvars live inside a Game, which can and does load and run
		many copies of world and lua code. CVars are global to the game, and
		therefore multiple runs of the same game script (i.e. level loads etc)
		will share the same cvars data.
		
		Functions:
		
		cvar CVar(name)
			- To bind an existing CVar.
			* Lua code will die with an error if the specified CVar does not exist
			
		cvar CVarBool(name, value)
			- Find or creates a boolean cvar of the specified name.
			* If the cvar is created it is set to the specified default value
			
		cvar CVarInt(name, value)
			- Find or creates an integer cvar of the specified name.
			* If the cvar is created it is set to the specified default value
		
		cvar CVarFloat(name, value)
			- Find or creates a float cvar of the specified name.
			* If the cvar is created it is set to the specified default value
			
		cvar CVarString(name, value)
			- Find or creates a string cvar of the specified name.
			* If the cvar is created it is set to the specified default value
			
		cvar CVarFunc(name, "expression")
			- Find or creates a function cvar of the specified name.
			* If the cvar is created it is set to the specified default value
			
			NOTE: Lua CVarFunc's are exceptional cases in several ways. CVarFunc's created
			in lua cannot bind a function or a closure, but instead pass a string which can be
			evaluated as a function call visible from the global lua scope.
			
			Example:
				function MyGlobalCVarFunction(cmdline)
					COutLine(kC_Debug, cmdline)
				end
				
				cv_MyCVarFunc = CVarFunc("mycvarfunc", "MyGlobalCVarFunction")
				cv_MyCVarFunc:Execute("PrintThisOut")
				
---------------------------------------------------------------------------]]--

cv_scexec = CVarFunc("sc", "_cv_scexec_func")
cv_sciexec = CVarFunc("sci", "_cv_sciexec_func")
cv_r_speeds = CVarBool("r_speeds", false)
cv_load = CVarFunc("load", "_cv_load_func")
cv_loadcheckpoint = CVarFunc("loadcheckpoint", "_cv_loadcheckpoint_func")
cv_savechecpoint = CVarFunc("savecheckpoint", "_cv_savecheckpoint_func")
cv_postevent = CVarFunc("postevent", "_cv_postevent_func")
cv_r_fly = CVarBool("r_fly", false)
cv_godmode = CVarFunc("godmode", "_cv_godmode_func")
cv_drg = CVarFunc("drg", "_cv_debug_reflex_game")
cv_dmg = CVarFunc("dmg", "_cv_debug_memory_game")
-------------------------------------------------------------------------------

function tabstr(tabs)
	local s = ""
	while (tabs > 0) do
		s = s.."\t"
		tabs = tabs - 1
	end
	
	return s
end

function _inspect(v, tab, str)

	if (tab == nil) then
		tab = 0
	end
	
	if (str == nil) then
		str = ""
	end
	
	if (v == nil) then
		str = str.."nil"
	else
		local t = type(v)
		
		if ((t == "number") or (t == "boolean")) then
			str = str..tostring(v)
		elseif (t == "string") then
			str = str.."\""..v.."\""
		elseif (t == "table") then
			str = str.."{\n"
			local indent = tabstr(tab+1)
			local prev = false
			for k, _v in pairs(v) do
				if (prev) then
					str = str..",\n"
				end
				if (type(k) == "string") then
					str = str..indent.."\""..k.."\" = "
				else
					str = str..indent..tostring(k).." = "
				end
				str = _inspect(_v, tab+1, str)
				prev = true
			end
			if (prev) then
				str = str.."\n"
			end
			str = str..tabstr(tab).."}"
		else
			str = str..t
		end
		
	end
	
	return str
end

function inspect(v)
	local str = _inspect(v)
	COut(kC_Debug, "\n"..str.."\n")
end

function _cv_scexec_func(cmd)
	if (cmd == nil) then
		error("expected argument!")
	end
	if (SCExecContext.f) then
		error("busy running another command!")
	end
	f, e = loadstring(cmd)
	if (f == nil) then
		error(e)
	end
	SCExecContext.f = f
	SCExecContext.entity.think = SCExecContext.Think
	SCExecContext.entity:SetNextThink(0)
end

function _cv_sciexec_func(cmd)
	if (cmd == nil) then
		error("expected argument!")
	end
	f, e = loadstring(cmd)
	if (f == nil) then
		error(e)
	end
	f()
end

function _cv_load_func(cmd)
	World.RequestLoad(cmd, kUnloadDisposition_Slot)
end

function _cv_loadcheckpoint_func()
	Abducted.entity:DoLoadCheckpoint()
end

function _cv_savecheckpoint_func()
	Abducted.entity.showCheckpointNotification = true
	World.RequestGenerateSaveGame()
end

function _cv_postevent_func(cmd)
	World.PostEvent(cmd)
end

function _cv_godmode_func(cmd)
	PlayerPawn.GodMode = not PlayerPawn.GodMode
	if (PlayerPawn.GodMode) then
		COutLine(kC_Debug, "godmode ON")
	else
		COutLine(kC_Debug, "godmode OFF")
	end
end

function _cv_debug_reflex_game()
	ReflexGame:DebugStart()
end

function _cv_debug_memory_game()
	MemoryGame:DebugStart()
end