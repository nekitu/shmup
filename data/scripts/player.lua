local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end