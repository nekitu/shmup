local C = {}

function C:init(unit)
  self.unit = unit
  self.activeTime = unit:getFloat("activeTime")
end

function C:onUpdate()
  self.time = self.time + game.deltaTime
end

function C:onCollide(other, colPairs)
  if other.unitResource.unitType == UnitType_Player then
    self.unit.deleteMeNow = true
  end
end

return newInstance(C)
