-- System.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
		System Level Engine Support Code
-----------------------------------------------------------------------------]]

-- CoResults
kCoResultPending = 0
kCoResultSuccess = 1
kCoResultError = 2

if LUA_VERSION_NUM >= 502 then
	-- lua 5.2 missing unpack()
	function unpack(x)
		if (x) then
			return table.unpack(x)
		end
		return nil
	end
end

--[[---------------------------------------------------------------------------
		Platform Helpers
-----------------------------------------------------------------------------]]

function System.IsPC()
	local plat = System.Platform()
	return (plat == kPlatMac) or (plat == kPlatWin)
end

function System.IsConsole()
	local plat = System.Platform()
	return (plat == kPlatXBox360) or (plat == kPlatPS3)
end

function  System.IsIOS()
	local plat = System.Platform()
	return (plat == kPlatIPad) or (plat == kPlatIPhone)
end


--[[---------------------------------------------------------------------------
		Entity Spawn / Load Tasks
-----------------------------------------------------------------------------]]

--[[ 
World.CreateEntity
	Sets up entity system table
--]]

function World.CreateEntity(entity, id, _ptr)
	
	entity.id = id
	
	entity.sys = { 
		ptr = _ptr, 
		state = nil,
		interrupt = nil,
		complete = nil,
		callbacks = {}
	}
	
	return entity
		
end

--[[ 
World.RunCo
	Executes an entity corotuine if one exists
--]]

function World.RunCo(entity)
	if (entity.sys.task) then
		if (entity.sys.coerr) then
			return kCoResultError
		end
		
		World.coroutine_entity = entity
		
		local r, err = coroutine.resume(entity.sys.task)
		if (r) then
			r = coroutine.status(entity.sys.task) ~= "dead"
			if not (r) then 
				entity.sys.task = nil
			end
		else
			entity.sys.coerr = true
			World.coroutine_entity = nil
			COutLine(kC_Error, "Script Error: %s", err)
			return kCoResultError
		end
		
	end
	
	World.coroutine_entity = nil
	
	if (entity.sys.task) then
		return kCoResultPending
	end
	return kCoResultSuccess	
end

--[[
World.CoSpawn
	Sets the entities current coroutine to spawn.
--]]
function World.CoSpawn(entity, keys)
	entity.keys = keys
	entity.sys.task = coroutine.create(function () entity:Spawn() end)
end

--[[
World.CoSpawn
	Sets the entities current coroutine to postspawn.
--]]
function World.CoPostSpawn(entity)
	if entity.PostSpawn then
		entity.sys.task = coroutine.create(function () entity:PostSpawn() end)
	else
		entity.sys.task = nil
	end
end

--[[
World.CoThink
	Sets the entities current coroutine to think.
--]]

function World.CoThink(entity)
	local f = function ()
		local time = World.GameTime()
		while true do
			local dt = World.GameTime() - time;
			if entity.think then
				entity.think(entity, dt)
			end
			if not entity.think then
				entity:SetNextThink(1000000) -- don't call back in here
			end
			time = World.GameTime()
			coroutine.yield()
		end
	end
	entity.sys.task = coroutine.create(f)
end

--[[
World.Precache [ THINK ONLY ]
	Precaches an asset.
	
	A precached asset is a data-only reference, no engine side classes are created.
	Precaching an asset is useful for resources that may be loaded via World.Load() during
	runtime (other the first World.Load() call make take many frames).
--]]

function World.Precache(path, async)
	COutLine(kC_Debug, "Precaching -- %s", path)
	
	local entity = World.coroutine_entity
	if (entity == nil) then
		COutLine(kC_Error, "ERROR: World.Precache() was not called from an entity coroutine!", path)
		return nil
	end
	
	if (async == nil) then
		async = true
	end
	
	local state = System.CreatePrecacheTask(entity, path, false, 0, async)
	if (not state) then
		COutLine(kC_Warn, "(File Not Found) FAILED to precache %s.", path)
		return nil
	end

	while (state:Pending()) do
		coroutine.yield()
	end
	
	local r = state:Result()
	
	if (r) then
		COutLine(kC_Debug, "Finished Precaching -- %s", path)
	else
		COutLine(kC_Warn, "FAILED to precache %s", path)
	end
	
	return r
end

--[[
World.Load [ THINK ONLY ]
	Loads an asset.
--]]

function World.Load(path, numInstances, async)

	local entity = World.coroutine_entity

	if (entity == nil) then
		COutLine(kC_Error, "ERROR: World.Load() was not called from an entity coroutine!", path)
		return nil
	end
	
	-- for sounds
	if (numInstances == nil) then
		numInstances = 1
	end
	
	if (async == nil) then
		async = true
	end
	
	COutLine(kC_Debug, "Loading -- %s", path)
	local state = System.CreatePrecacheTask(entity, path, true, numInstances, async)
	if (not state) then
		COutLine(kC_Warn, "(File Not Found) FAILED to load %s.", path)
		return nil
	end

	while (state:Pending()) do
		coroutine.yield()
	end
	
	local r = state:Result()
	
	if (r) then
		COutLine(kC_Debug, "Finished Loading -- %s", path)
	else
		COutLine(kC_Warn, "FAILED to load %s", path)
	end
	
	return r
end

--[[
World.Load [ THINK ONLY ]
	Loads a sound and sets default distances.
--]]

function World.LoadSound(path, numInstances, async, refDistance, maxDistance)
	
	if (World.coroutine_entity == nil) then
		COutLine(kC_Error, "ERROR: World.LoadSound() was not called from an entity coroutine!", path)
		return nil
	end
	
	local sound = World.Load(path, numInstances, async)
	if (sound) then
		if (refDistance == nil) then
			refDistance = 150
		end
		if (maxDistance == nil) then
			maxDistance = 300
		end
		
		sound:SetRefDistance(refDistance)
		sound:SetMaxDistance(maxDistance)
	end
	
	return sound
	
end

--[[
World.Spawn [ THINK ONLY ]
	Spawns an entity
--]]

function World.Spawn(keys)

	local entity = World.coroutine_entity
	
	if (entity == nil) then
		COutLine(kC_Error, "ERROR: World.Spawn() was not called from an entity coroutine!", path)
		return nil
	end

	COutLine(kC_Debug, "Spawning -- %s", keys.classname)
	local state = System.CreateSpawnTask(entity, keys)
	
	while (state:Pending()) do
		coroutine.yield()
	end
	
	local r = state:Result()
	
	if (r) then
		COutLine(kC_Debug, "Finished Spawning -- %s", keys.classname)
	else
		COutLine(kC_Warn, "FAILED to spawn %s", keys.classname)
	end
	
	return r
end

--[[
World.TempSpawn [ THINK ONLY ]
	Spawn a temporary entity
--]]

function World.TempSpawn(keys)

	local entity = World.coroutine_entity
	
	if (entity == nil) then
		COutLine(kC_Error, "ERROR: World.TempSpawn() was not called from an entity coroutine!", path)
		return nil
	end
	
	COutLine(kC_Debug, "Spawning -- %s", keys.classname)
	local state = System.CreateTempSpawnTask(entity, keys)
	
	while state:Pending() do
		coroutine.yield()
	end
	
	local r = state:Result()
	
	if r then
		COutLine(kC_Debug, "Finished Spawning -- %s", keys.classname)
	else
		COutLine(kC_Warn, "FAILED to spawn %s", keys.classname)
	end
	
	return r
end

--[[
World.Pause
	Pauses the game
--]]

function World.PauseGame(pause)

	World.paused = pause
	if pause then
		World.SetPauseState(bit.bor(PauseGame, PauseCinematics))
	else
		World.SetPauseState(0)
	end

end

--[[---------------------------------------------------------------------------
		Called By Engine
-----------------------------------------------------------------------------]]

function World.NotifyBackground()
	if World.game and World.game.NotifyBackground then
		World.game:NotifyBackground()
	end
end

function World.NotifyResume()
	if World.game and World.game.NotifyResume then
		World.game:NotifyResume()
	end
end

function World.SaveApplicationState()
	if World.game and World.game.SaveApplicationState then
		World.game:SaveApplicationState()
	end
end

function World.RestoreApplicationState()
	if World.game and World.game.RestoreApplicationState then
		World.game:RestoreApplicationState()
	end
end

