-- Math.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
		Math Library
-----------------------------------------------------------------------------]]

--[[---------------------------------------------------------------------------
		Utils
-----------------------------------------------------------------------------]]

function Lerp(a, b, t)
	return  a + (b-a)*t
end

function FloatRand(a, b)
	if (a ~= nil) and (b ~= nil) then
		local x = Lerp(a, b, math.random())
		return x
	end
	return math.random()
end

function IntRand(a, b)

	if (a ~= nil) and (b ~= nil) then
		local x = Lerp(a, b, math.random())
		return math.floor(x + 0.5)
	end
	
	return 0
	
end

function DegToRad(degs)
	return degs / 180 * math.pi
end

function RadToDeg(rad)
	return rad / math.pi * 180
end

function Clamp(x, min, max)
	
	if (x < min) then
		return min
	end
	
	if (x > max) then
		return max
	end
	
	return x
end

function Min(x, min)
	if x < min then
		return x
	end
	return min
end

function Max(x, max)
	if x > max then
		return x
	end
	return max
end

function Abs(x)
	return ((x<0) and (-x)) or x
end

function LookAngles(vec)
	local yaw = math.atan2(vec[2], vec[1])
	local pitch = math.asin(-vec[3])
	v = { 0, RadToDeg(pitch), RadToDeg(yaw) }
	
	for i=1,3 do
		if v[i] < 0 then
			v[i] = v[i] + 360
		end
		if v[i] > 360 then
			v[i] = v[i] - 360
		end
	end
	
	return v
end

function WrapAngles(angles)
	local wrap = { 0, 0, 0 }
	for i=1,3 do
		wrap[i] = math.fmod(angles[i], 360)
	end
	return wrap
end

function DeltaAngles(x, y) -- returns shortest path
	local delta = { 0, 0, 0 }
	for i=1,3 do
		local xx = x[i]
		local yy = y[i]
		if (xx < 0) then
			xx = xx + 360
		end
		if (yy < 0) then
			yy = yy + 360
		end
		
		if (yy > xx and (yy-xx) > 180) then
			yy = yy - 360
		elseif (xx > yy and (xx-yy) > 180) then
			xx = xx - 360
		end
		
		delta[i] = yy-xx
	end
	
	return delta
end

function CapsuleDistance(p, line)

	local vec = VecSub(line[2], line[1])
	local vec, mag = VecNorm(vec)

	-- check off end
	local plane = MakePlane(vec, line[1])
	local distance = PlaneDistance(plane, p)
	
	if (distance < 0 or distance > mag) then
		-- radial distance
		if (distance < 0) then
			distance = VecMag(VecSub(p, line[1]))
		else
			distance = VecMag(VecSub(p, line[2]))
		end
	else
		local up = {0,0,1}
		if (vec[2] > 0.9) then
			up = {0,1,0}
		end

		local normal = VecCross(vec, up)
		plane = MakePlane(normal, line[1])
		distance = PlaneDistance(plane, p)
	end

	return distance

end

function QuatFrameFromAngles(angles)
	local f = QuatFromAxis
	local qx = f({1,0,0}, angles[1])
	local qy = f({0,1,0}, angles[2])
	local qz = f({0,0,1}, angles[3])
	return qx, qy, qz
end

function VecsFromQuatFrame(qx, qy, qz)
	local rotate = QuatRotateVec
	local x = rotate(qx, {1,0,0})
	x = rotate(qy, x)
	x = rotate(qz, x)
	local y = rotate(qx, {0,1,0})
	y = rotate(qy, y)
	y = rotate(qz, y)
	local z = rotate(qx, {0,0,1})
	z = rotate(qy, z)
	z = rotate(qz, z)
	return x, y, z
end

function VecsFromAngles(angles)
	local qx, qy, qz = QuatFrameFromAngles(angles)
	return VecsFromQuatFrame(qx, qy, qz)
end

function ForwardVecFromAngles(angles)
	local qx, qy, qz = QuatFrameFromAngles(angles)
	local rotate = QuatRotateVec
	local x = rotate(qx, {1,0,0})
	x = rotate(qy, x)
	x = rotate(qz, x)
	return x
end

--[[---------------------------------------------------------------------------
		Planes
-----------------------------------------------------------------------------]]

function MakePlane(normal, v)
	return { normal[1], normal[2], normal[3], VecDot(normal, v) }
end

function PlaneDistance(plane, v)
	return VecDot(plane, v) - plane[4]
end

function IntersectPlane(plane, v0, v1)

	local t0 = PlaneDistance(plane, v0)
	local t1 = PlaneDistance(plane, v1)
	
	if (t0 < 0 and t1 < 0) then
		return nil
	elseif (t0 >= 0 and t1 >= 0) then
		return nil
	end
	
	local d = t0 / (t0 - t1)
	return VecLerp(v0, v1, d)

end

--[[---------------------------------------------------------------------------
		Ranges
-----------------------------------------------------------------------------]]

function InitRange()

	return { 999999, 999999, 999999 }, { -999999, -999999, -999999 }

end

function InsertRange(v, mins, maxs)

	for i=1,3 do
		if v[i] < mins[i] then
			mins[i] = v[i]
		end
		if v[i] > maxs[i] then
			maxs[i] = v[i]
		end
	end

end

function InRange(v, mins, maxs)

	for i=1,3 do
		if (v[i] < mins[i]) or (v[i] > maxs[i]) then
			return false
		end
	end
	
	return true

end

--[[---------------------------------------------------------------------------
		Vectors
-----------------------------------------------------------------------------]]

function XAxis()
	return { 1, 0, 0 }
end

function YAxis()
	return { 0, 1, 0 }
end

function ZAxis()
	return { 0, 0, 1 }
end

function VecInit(x, y, z)
	return { x, y, z }
end

function VecZero()
	return { 0, 0, 0 }
end

function VecEq(x, y, ep)
	if ep == nil or ep == 0 then
		return x[1] == y[1] and x[2] == y[2] and x[3] == y[3]
	end
	
	if x[1]+ep >= y[1] and x[1]-ep <= y[1] then
		if x[2]+ep >= y[2] and x[2]-ep <= y[2] then
			if x[3]+ep >= y[3] and x[3]-ep <= y[3] then
				return true
			end
		end
	end
	
	return false
end

function VecDot(x, y)
	return x[1] * y[1] + x[2] * y[2] + x[3] * y[3]
end

function VecSub(x, y)
	return { x[1] - y[1], x[2] - y[2], x[3] - y[3] }
end

function VecAdd(x, y)
	return { x[1] + y[1], x[2] + y[2], x[3] + y[3] }
end

function VecNeg(x)
	return { -x[1], -x[2], -x[3] }
end

function VecCross(x, y)
	return {
		x[2]*y[3] - x[3]*y[2],
		x[3]*y[1] - x[1]*y[3],
		x[1]*y[2] - x[2]*y[1]
	}
end

function VecSqMag(x)
	return VecDot(x, x)
end

function VecMag(x)
	return math.sqrt(VecDot(x, x))
end

function VecNorm(x)
	local len = VecMag(x)
	return { x[1] / len, x[2] / len, x[3] / len }, len
end

function VecMul(x, y)
	return { x[1]*y[1], x[2]*y[2], x[3]*y[3] }
end

function VecScale(x, s)
	return { x[1]*s, x[2]*s, x[3]*s }
end

-- a + s*b
function VecMA(a, s, b)
	return VecAdd(a, VecScale(b, s))
end

function VecLerp(a, b, s)
	local d = VecSub(b, a)
	return VecMA(a, s, d)
end

function OrthoVec(a, b)
	local z = VecCross(a, b)
	return VecNorm(z)
end

--[[---------------------------------------------------------------------------
		Quaternions
-----------------------------------------------------------------------------]]

function QuatIdentity()
	return { 0, 0, 0, 0 }
end

function QuatFromAxis(vec, rot)
	local q = { 0, 0, 0, 0 }
	rot = DegToRad(rot * 0.5)
	q[4] = math.cos(rot)
	local sin = math.sin(rot)
	q[1] = vec[1] * sin
	q[2] = vec[2] * sin
	q[3] = vec[3] * sin
	return q
end

function QuatToAxis(q)
	local theta = math.acos(q[4])
	if (theta < 0.00001) then
		return { 0, 0, 0 }, 0
	end
	local rot = RadToDeg(theta*2)
	theta = 1 / math.sin(theta)
	return { q[1]*theta, q[2]*theta, q[3]*theta }, rot
end

function QuatToAngles(q)
	local v = { 0, 0, 0 }
	local faz = 2 * (q[1]*q[3])+(q[2]*q[4])
	
	if faz > 1 then
		faz = 1
	end
	
	if faz < -1 then
		faz = -1
	end
	
	faz = -math.sin(-faz)
	v[2] = faz
	
	local num, dem
	local atan2 = math.atan2
	
	if math.cos(faz) ~= 0 then
		num = 2*(q[1]*q[2])-(q[2]*q[3])
		dem = 1-(2*((q[2]*q[2])-(q[3]*q[3])))
		v[3] = -atan2(num,dem)
		num = 2*((q[2]*q[3])+(q[1]*q[4]))
		dem = 1-(2*((q[1]*q[1])-(q[2]*q[2])))
	else
		v[3] = 0
		num = 2*((q[1]*q[2])+(q[3]*q[4]))
		dem = 1-(2*((q[1]*q[1])-(q[3]*q[3])))
	end
	
	v[1] = -atan2(num, dem)
	return v
end

function QuatFromAngles(angles)
	local cos = math.cos
	local sin = math.sin
	local sx = DegToRad(angles[1]*0.5)
	local sy = DegToRad(angles[2]*0.5)
	local sz = DegToRad(angles[3]*0.5)
	
	local cx = cos(sx)
	sx = sin(sx)
	local cy = cos(sy)
	sy = sin(sy)
	local cz = cos(sz)
	sz = sin(sz)
	
	local cycz = cy*cz
	local sysz = sy*sz
	local cysz = cy*sz
	local sycz = sy*cz
	
	return { sx*cycz+cx*sysz, cx*sycz+sx*cysz, cx*cysz+sz*sycz, cx*cycz+sx*sysz }
end
	

function QuatMul(qa, qb)
	local out = { 0, 0, 0, 0 }
	out[4] = ((qa[4]*qb[4])-(qa[1]*q2[1]))-((qa[2]*qb[2])-(qa[3]*qb[3]))
	out[1] = ((qa[4]*qb[1])+(qa[1]*q2[4]))+((qa[2]*qb[3])-(qa[3]*qb[2]))
	out[2] = ((qa[4]*qb[2])-(qa[1]*q2[3]))+((qa[2]*qb[4])+(qa[3]*qb[1]))
	out[3] = ((qa[4]*qb[3])+(qa[1]*q2[2]))-((qa[2]*qb[1])+(qa[3]*qb[4]))
	return out
end

function QuatInverse(q)
	local z = { 0, 0, 0, 0 }
	z[1] = -q[1]
	z[2] = -q[2]
	z[3] = -q[3]
	z[4] = q[4]
	return z
end

function QuatRotateVec(q, v)
	local qw = -(q[1]*v[1]) - (q[2]*v[2]) - (q[3]*v[3])
	local qx =  (q[4]*v[1]) + (q[2]*v[3]) - (q[3]*v[2])
	local qy =  (q[4]*v[2]) - (q[1]*v[3]) + (q[3]*v[1])
	local qz =  (q[4]*v[3]) + (q[1]*v[2]) - (q[2]*v[1])
	v[1] = -(qw*q[1]) + (qx*q[4]) - (qy*q[3]) + (qz*q[2])
	v[2] = -(qw*q[2]) + (qx*q[3]) + (qy*q[4]) - (qz*q[1])
	v[3] = -(qw*q[3]) - (qx*q[2]) + (qy*q[1]) + (qz*q[4])
	return v
end
