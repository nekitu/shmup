local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onAppeared()
  self.unit.persistentOnChangeMap = true
  game:changeMapByIndex(1)
end

return newInstance(C)
