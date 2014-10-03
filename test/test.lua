--test.lua
local x = 5
local f = 5.37
local str = "hello world\n"

x = 3 + 4

for i = 1, 10, 1 do
	x = x + 1
end

local run = true

while run do
	x = x + 1
end

repeat
	x = x + 1
until x > 30

if x > 40 then
	x = 39
elseif x > 30 then
	x = 31
else
	x = 29
end

--[[
	a bunch
	of random
	stuff
	comments
]]
