local C = {}

function C:init(unit)
  self.unit = unit
  self.time = 0
end

function C:onUpdate()
  self.unit.root.position:add(Vec2(math.sin(self.time)/20, 0))
  self.time = self.time + game.deltaTime
end

function C:onCollide(other, colPairs)
  if other.unitResource.unitType == UnitType_Player then
    --todo: dont delete if hit by weapon projectile or beam
    self.unit.deleteMeNow = true
  end
end

return newInstance(C)
