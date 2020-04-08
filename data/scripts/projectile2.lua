local C = {}

function C:init(unit)
  self.unit = unit
  self.controllers = {}
  local ctrl = require("controllers/projectile")(unit)
  table.insert(self.controllers, ctrl)
end

function C:onUpdate()
  for _, c in ipairs(self.controllers) do
    c:update()
  end
end

function C:onCollide(other, collisions)
  if true then return end
  local colCenter = Vec2(0, 0)
  if self.unit.unitResource.type == 4 and self.unit.root:checkPixelCollision(other.root, colCenter) then
    self.unit.deleteMeNow = true
    if other.unitResource.type == 5 then other.deleteMeNow = true end
  game:spawn("units/small_spark", "expl", colCenter)
  other.root:hit(5)
  end
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end