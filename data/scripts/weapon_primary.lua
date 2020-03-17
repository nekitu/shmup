local C = {}

function C:init(weapon)
  self.weapon = weapon
end

function C:onFire()
end

return function(obj)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(obj)
  return o
end