-- Persistence.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Persistence = {}

function Persistence.MakePath(key, args)

	local s = nil
	
	if args then
		for k,v in pairs(args) do
			if s ~= nil then
				s = s.."/"..tostring(v)
			else
				s = tostring(v)
			end
		end
	end
	
	if s ~= nil then
		return tostring(key).."/"..s
	end
	
	return tostring(key)

end

function Persistence.TablePath(tab, path)

	path = string.split(path, "/")
	
	for i = 1, (#path-1) do
		local n = tab[path[i]]
		if (n == nil) then
			n = {}
			tab[path[i]] = n
		end
		tab = n
	end
		
	return tab, path[#path]

end

function Persistence.DeleteKey(storage, key, ...)
	local path = Persistence.MakePath(key, {...})

	local tab
	tab, key = Persistence.TablePath(storage.keys, path)
	
	tab[key] = nil
end

function Persistence.ReadString(storage, key, default, ...)
	local path = Persistence.MakePath(key, {...})

	local tab
	tab, key = Persistence.TablePath(storage.keys, path)
		
	return StringForString(tab[key], default)

end


function Persistence.WriteString(storage, key, value, ...)
	local path = Persistence.MakePath(key, {...})

	local tab
	tab, key = Persistence.TablePath(storage.keys, path)
	
	tab[key] = tostring(value)
end

function Persistence.ReadNumber(storage, key, default, ...)

	return NumberForString(Persistence.ReadString(storage, key, nil, ...), default)

end

function Persistence.WriteNumber(storage, key, value, ...)

	Persistence.WriteString(storage, key, tostring(value), ...)

end

function Persistence.ReadVec3(storage, key, default, ...)

	return Vec3ForString(
		Persistence.ReadString(storage, key, nil, ...),
		default
	)

end

function Persistence.WriteVec3(storage, key, value, ...)

	Persistence.WriteString(
		storage,
		key,
		string.format("%d %d %d", value[1], value[2], value[3]),
		...
	)

end

function Persistence.ReadColor4(storage, key, default, ...)

	local s = Persistence.ReadString(
		storage,
		key,
		nil,
		...
	)
	
	return Color4ForString(s, default)

end

function Persistence.WriteColor4(storage, key, value, ...)

	Persistence.WriteString(
		storage,
		key,
		Color4ToString(value),
		...
	)

end

function Persistence.ReadBool(storage, key, default, ...)

	return BoolForString(Persistence.ReadString(storage, key, nil, ...), default)		

end

function Persistence.WriteBool(storage, key, value, ...)
	
	Persistence.WriteString(storage, key, value, ...)

end
