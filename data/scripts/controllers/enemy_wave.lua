local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
  self.moverType = params.getString("moverType")
  if self.moverType == "line" then
    self.lineDirection = params.getVec2("lineDirection")
  end
end

function C:onUpdate()
  if not self.unit.root then return end
  if self.moverType == "line" then
    self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime
  end
end

return newInstance(C)