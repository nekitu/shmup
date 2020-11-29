require("util")

local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
end

return newInstance(C)