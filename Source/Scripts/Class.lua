-- Class.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Class = {}

function Class:New(x)
	local x = x or {}
	setmetatable(x, self)
	self.__index = self
	return x
end

