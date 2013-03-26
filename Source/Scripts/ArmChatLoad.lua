-- ArmChatLoad.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Load available conversations
-----------------------------------------------------------------------------]]

Arm.Chats.Available = {}

function Arm.PopulateChats(self, group)
	for k,v in pairs(Arm.Chats) do
		for a,b in pairs(v) do
			if (b.group == group) then
				Arm.Chats.Available[a] = b
			end
		end
	end
end

function Arm.LoadCommonChat(self)
    Arm.Chats.CommonDB = World.Load("UI/ArmChat")
    local strings = Arm.Chats.CommonDB:StringTable()
    Arm.Chats.CommonDB = Arm.Chats.CommonDB:Data()
    Arm.Chats.CommonDB.StringTable = strings
    
    for k,v in pairs(Arm.Chats.CommonDB) do
		v.stringTable = strings
        if ((v.group == nil) or (v.group == "Default")) then
            Arm.Chats.Available[k] = v
        end
    end
end

function Arm.LoadChatList(self, chats)
	chats = string.split(chats, ";")
	
	for k,v in pairs(chats) do
		local db = World.Load(v)
		local strings = db:StringTable()
		db = db:Data()
		if (db) then
			for a,b in pairs(db) do
				b.stringtable = strings
				if ((b.group == nil) or (b.group == "Default")) then
					Arm.Chats.Available[a] = b
				end
			end
		end
	end
    
end

