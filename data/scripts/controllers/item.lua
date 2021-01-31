local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(ctrlInfo)
  self.itemType = ctrlInfo:getString("itemType", "")
  self.value = ctrlInfo:getFloat("value", 0)
end

function C:onUpdate()
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end
