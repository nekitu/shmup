local C = {}

function C:init(weapon)
  self.weapon = weapon
end

function C:onFire()
end
local s = 0
function C:onUpdate()
  --self.weapon.fireAngleOffset = 0-- math.sin(s) * 40
  --s = s + 1*game.deltaTime
end

return function(obj)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(obj)
  return o
end