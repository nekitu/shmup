local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
end

function C:onCollide(other)
end

return newInstance(C)
