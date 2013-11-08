-- ArmChatLoad.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Load available conversations
-----------------------------------------------------------------------------]]
kArmChatFlag_Locked = 1
kArmChatFlag_AutoGenerate = 2
kArmChatFlag_Procedural = 4
kArmChatFlag_ShuffleChoices = 8

Arm.Chats.Available = {}
Arm.Chats.Loaded = {}
Arm.Chats.Procedural = {}

function Arm.LinkDialogDB(self, db)

	local strings = db:StringTable()
	local data = db:Data()
	
	if (data) then
		data.stringTable = strings
		
		for k,v in pairs(data.roots) do
			if (Arm.Chats.Loaded[k] ~= nil) then
				error(string.format("Arm topic '%s' is defined multiple times.", k))
			end
			Arm.Chats.Loaded[k] = v
			v.stringTable = strings
			
			if (v.group) then
				v.group = string.split(v.group, ";")
				if (#v.group == 0) then
					v.group = nil
				end
			end
			
			Arm:SetTopicFlags(k, v)
			Arm:LinkProceduralChat(k, v)
		end
		
		for k,v in pairs(data.dialogs) do
			v.stringTable = strings
			if (v.group) then
				v.group = string.split(v.group, ";")
				if (#v.group == 0) then
					v.group = nil
				end
			end
		end
	end

	return data
end

function Arm.FindChatString(self, text)
	for k,v in pairs(Arm.Chats.Loaded) do
		local z = StringTable.Get(text, v.stringTable, true)
		if (z) then
			return z
		end
	end
	return StringTable.Get(text)
end

function Arm.TopicIsProcedural(self, topic)
	return bit.band(topic.flags, kArmChatFlag_Procedural) ~= 0
end

function Arm.LinkProceduralChat(self, name, topic)

	if (bit.band(topic.flags, kArmChatFlag_Procedural) == 0) then
		return
	end
	
	if (bit.band(topic.flags, kArmChatFlag_Locked) ~= 0) then
		return -- locked
	end
	
	if (topic.priority <= 0) then
		return
	end
	
	local t = Arm.Chats.Procedural[topic.priority]
	if (t == nil) then
		t = {}
		Arm.Chats.Procedural[topic.priority] = t
	end
	
	if (#topic.choices ~= 1) then
		error(string.format("Arm topic '%s' is a procedural topic and must contain 1 dialog.", name))
	end
	
	local choice = topic.choices[1]
	choice.topicGroup = topic.group
	
	table.insert(t, choice)
end

function Arm.SortProceduralTopics(self, priority, groups)

	if (groups == nil) then
		return nil
	end
	
	local topics = Arm.Chats.Procedural[priority]
	if (topics ~= nil) then
		local final = {}
		
		for k,root in pairs(topics) do
			root.used = false
		end
		
		for k,group in pairs(groups) do
		-- add in group order
			local filtered = {}
			for k,root in pairs(topics) do
				if (root.topicGroup) then
					if (not root.used) then
						if (FindArrayElement(root.topicGroup, group)) then
							root.used = true
							table.insert(filtered, root)
						end
					end
				end
			end	
			filtered = ShuffleArray(filtered)
			for k,v in pairs(filtered) do
				table.insert(final, v)
			end			
		end
		
		topics = final
	end
	return topics
end

function Arm.UnlockTopic(self, name, topic)
	if (topic == nil) then
		topic = Arm.Chats.Loaded[name]
		if (topic == nil) then
			COutLine(kC_Error, "ERROR: Arm topic '%s' does not exist or is not loaded.", name)
			return false
		end
	end
	
	local locked = bit.band(topic.flags, kArmChatFlag_Locked) ~= 0
	if (locked) then
	
		topic.flags = bit.band(topic.flags, bit.bnot(kArmChatFlag_Locked))
		Persistence.WriteBool(SaveGame, "armTopicUnlocked", true, topic.name)
		
		if (bit.band(topic.flags, kArmChatFlag_Procedural) == 0) then
			if (topic.priority <= 0) then
				COutLine(kC_Error, "ERROR: Arm topic '%s' is a critical path topic (priority <= 0), it cannot be unlocked, it must be triggered.", topic.name)
				return false
			end
			Arm.Chats.Available[name] = topic
		else
			Arm:LinkProceduralChat(topic.name, topic)
		end
		
		EventLog:AddEvent(
			GameDB:ArmDateString(),
			"!TOPIC",
			topic.name
		)
		
		return true
	end
	
	return false
end

function Arm.SetTopicFlags(self, name, topic)

	local unlocked = Persistence.ReadBool(SaveGame, "armTopicUnlocked", false, name)
	if (unlocked) then
		topic.flags = bit.band(topic.flags, bit.bnot(kArmChatFlag_Locked))
	end

end

function Arm.CheckTopicReward(self, topic, type)
	return Persistence.ReadBool(SaveGame, "armTopicRewards", false, name, type)
end

function Arm.SaveTopicReward(self, topic, type)
	Persistence.WriteBool(SaveGame, "armTopicRewards", true, name, type)
end

function Arm.AddAvailableChats(self, db, flags)

	if (flags == nil) then
		flags = 0
	end
	
	for k,v in pairs(db.roots) do
		if (v.priority > 0) then
			local skip = false
				
			if (bit.band(v.flags, kArmChatFlag_Locked) ~= 0) then
				if (bit.band(flags, kArmChatFlag_Locked) ~= 0) then
					Arm:UnlockTopic(k, v)
				else
					skip = true
				end
			end
			
			if (bit.band(v.flags, kArmChatFlag_Procedural) ~= 0) then
				skip = true
			end
			
			if (not skip) then
				Arm.Chats.Available[k] = v
			end
		end
	end

end

function Arm.LoadCommonChat(self)
    Arm.Chats.CommonDB = World.Load("UI/ArmChat")
    Arm.Chats.CommonDB = Arm:LinkDialogDB(Arm.Chats.CommonDB)
    
    if (Arm.Chats.CommonDB) then
		Arm:AddAvailableChats(Arm.Chats.CommonDB)
	end
end

function Arm.LoadChatList(self, chats)
	chats = string.split(chats, ";")
	
	for k,v in pairs(chats) do
		local db = World.Load(v)
		db = Arm:LinkDialogDB(db)
		if (db) then
			Arm:AddAvailableChats(db)
			Arm.Chats[v] = db
		end
	end
    
end

