-- ArmChatLoad.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Load available conversations
-----------------------------------------------------------------------------]]

Arm.Chats.Available = {}

function Arm.PopulateChats(self, condition)

end

function Arm.LoadChatData(self)
    Arm.Chats.DB = World.Load("UI/ArmChat")
    Arm.Chats.Strings = Arm.Chats.DB:StringTable()
    Arm.Chats.DB = Arm.Chats.DB:Data()
    
    for k,v in pairs(Arm.Chats.DB) do
        if ((v.group == nil) or (v.group == "Default")) then
            Arm.Chats.Available[k] = v
        end
    end
end

