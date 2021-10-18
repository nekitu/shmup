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

function C:onSerialize(data)
  data.unitId = self.unit.id
  data.lineDirectionX = self.lineDirection.x
  data.lineDirectionY = self.lineDirection.y
end

function C:onDeserialize(data)
  self.unit = game:findUnitById(data.unitId)
  self.lineDirection = Vec2(data.lineDirectionX, data.lineDirectionY)
end

return newInstance(C)