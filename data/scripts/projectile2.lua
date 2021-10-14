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
  game:spawn("units/small_spark", "expl", colCenter)
end

return newInstance(C)