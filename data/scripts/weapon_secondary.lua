local C = {}

function C:init(weapon)
  self.weapon = weapon
end

function C:onFire()
end

function C:onUpdate()
end

return newInstance(C)