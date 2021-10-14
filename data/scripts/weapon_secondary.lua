local C = {}

function C:init(weapon)
  self.weapon = weapon
end

function C:onFire()
  print("fire" .. self.weapon.params.fireRate)
end

function C:onUpdate()
end

return newInstance(C)