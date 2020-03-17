local C = {}

function C:init()
end

function C:onUpdate()
end

function C:onRender(layerIndex)
end

function C:onScreenEnter()
end

function C:onScreenLeave()
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end