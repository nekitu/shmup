local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
  if not self.unit.root then return end
  self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end