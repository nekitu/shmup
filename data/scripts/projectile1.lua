local C = {}

function C:init(unit)
  self.unit = unit
  self.controllers = {}
  local ctrl = require("controllers/projectile")(unit)
  table.insert(self.controllers, ctrl)
end

function C:onUpdate()
end

function C:onCollide(other)
  local pos = Vec2(0, 0)
  if self.unit.root:checkPixelCollision(other.root, pos) then
    other.root:hit(1)
    print("HIT")
  end
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end
