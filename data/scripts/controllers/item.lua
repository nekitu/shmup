local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
  self.itemType = params:getString("itemType", "")
  self.value = params:getFloat("value", 0)
end

function C:onUpdate()
end

function C:onSerialize(data)
  data.unitId = self.unit.id
end

function C:onDeserialize(data)
  self.unit = unitFromId(data.unitId)
end

return newInstance(C)
