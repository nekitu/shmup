local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
  if not self.unit.root then return end
  self.unit.root.position = self.unit.root.position + self.unit.velocity:mulScalarReturn(self.unit.speed * game.deltaTime)
  self.unit.speed = self.unit.speed + self.unit.speed * self.unit.acceleration * game.deltaTime
  util.clampValue(self.unit.speed, self.unit.minSpeed, self.unit.maxSpeed)
end

function C:onSerialize(data)
  data.unitId = self.unit.id
end

function C:onDeserialize(data)
  self.unit = unitFromId(data.unitId)
end

return newInstance(C)