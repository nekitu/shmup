local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
  self.lineDirection = params:getVec2("lineDirection", Vec2(0, 1))
end

function C:onUpdate()
  if not self.unit.root then return end
  self.unit.root.position.x = self.unit.root.position.x + self.unit.speed * game.deltaTime * self.lineDirection.x
  self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime * self.lineDirection.y
end

return newInstance(C)