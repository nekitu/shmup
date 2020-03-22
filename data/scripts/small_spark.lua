local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate(unit)
  if self.unit.age > 0.5 then
    self.unit.deleteMeNow = true
  end
end

function C:onCollide(other)
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end