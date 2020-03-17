local M = {}

local function onUpdate(unit)
  if unit.age > 0.5 then
    unit.deleteMeNow = true
  end
end

local function onCollide(unitInst1, unitInst2)
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end