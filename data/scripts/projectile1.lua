local C = {}

function C:init(unit)
  self.unit = unit
  self.controllers = {}
  local ctrl = require("controllers/projectile")(unit)
  table.insert(self.controllers, ctrl)
end

function C:onUpdate()
end

function C:onCollide(other, cols)
end

return newInstance(C)