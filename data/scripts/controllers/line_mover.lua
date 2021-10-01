local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
  self.lineDirection = params.getVec2("lineDirection")
end

function C:onUpdate()
  if not self.unit.root then return end
  self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime
end

return newInstance(C)