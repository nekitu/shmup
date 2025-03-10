local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
  if not self.unit.root then return end
  self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime
end

function C:onSerialize(data)
end

function C:onDeserialize(data)
end

return newInstance(C)