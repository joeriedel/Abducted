-- Persistence.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Persistence = {}

function Persistence.MakePath(key, args)

	local s = nil
	
	if args then
		for k,v in pairs(args) do
			if (type(k) == "number") then
				if s ~= nil then
					s = s.."_"..tostring(v)
				else
					s = tostring(v)
				end
			end
		end
	end
	
	if s ~= nil then
		return s.."_"..tostring(key)
	end
	
	return tostring(key)

end

function Persistence.DeleteKey(storage, key, ...)
	local s = Persistence.MakePath(key, arg)
	storage.keys[s] = nil
end

function Persistence.ReadString(storage, key, default, ...)

	local s = Persistence.MakePath(key, arg)
	return StringForString(storage.keys[s], default)

end


function Persistence.WriteString(storage, key, value, ...)

	local s = Persistence.MakePath(key, arg)
	storage.keys[s] = tostring(value)

end

function Persistence.ReadNumber(storage, key, default, ...)

	return NumberForString(Persistence.ReadString(storage, key, nil, unpack(arg)), default)

end

function Persistence.WriteNumber(storage, key, value, ...)

	Persistence.WriteString(storage, key, tostring(value), unpack(arg))

end

function Persistence.ReadVec3(storage, key, default, ...)

	return Vec3ForString(
		Persistence.ReadString(storage, key, nil, unpack(arg)),
		default
	)

end

function Persistence.WriteVec3(storage, key, value, ...)

	Persistence.WriteString(
		storage,
		key,
		string.format("%d %d %d", value[1], value[2], value[3]),
		unpack(arg)
	)

end

function Persistence.ReadColor4(storage, key, default, ...)

	local s = Persistence.ReadString(
		storage,
		key,
		nil,
		unpack(arg)
	)
	
	return Color4ForString(s, default)

end

function Persistence.WriteColor4(storage, key, value, ...)

	Persistence.WriteString(
		storage,
		key,
		Color4ToString(value),
		unpack(arg)
	)

end

function Persistence.ReadBool(storage, key, default, ...)

	return BoolForString(
		Persistence.ReadString(storage, key, nil, unpack(arg)),
		default
	)		

end

function Persistence.WriteBool(storage, key, value, ...)
	
	Persistence.WriteString(storage, key, value, unpack(arg))

end
