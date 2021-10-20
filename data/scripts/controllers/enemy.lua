local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
  if not self.unit.root then return end
end

function C:onSerialize(data)
end

function C:onDeserialize(data)
end

return newInstance(C)