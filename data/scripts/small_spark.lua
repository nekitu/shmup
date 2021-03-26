local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate(unit)
  if self.unit.age > 0.5 then
    self.unit.deleteMeNow = true
  end
end

function C:onCollide(other)
end

return newInstance(C)