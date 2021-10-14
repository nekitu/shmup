local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
  if self.unit.age > 5 then
    self.unit.deleteMeNow = true
    print("deleting explosion unit")
  end
end

return newInstance(C)