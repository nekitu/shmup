local C = {}

function C:onStageChange(old, new)
	print("Change stage to "..new .. " from " .. old)
	if new == "stage0" then
		print("ACTIVATING MAGEBALAST")
		self.unit:findWeapon("gun1").active = true
	end
	if new == "stage1" then
		self.unit:findWeapon("gun2").active = true
    print("ACTIVATING LAST RESORT SWORD")
  end
end

function C:onUpdate()
	if self.unit.health == 0 then
    self.unit.deleteMeNow = true
    local uinst = game.spawn("units/turret_expl", "expl2", self.unit.root.position)
    uinst.layerIndex = self.unit.layerIndex
		game.animateCameraSpeed(110, 0.6)
		game.shakeCamera(Vec2(10, 10), 3, 200)
	end
end

function C:onCollide(other)
  local pos = Vec2(0, 0)
  if self.unit.root:checkPixelCollision(other.root, pos) then
  end
  local cols = {}
  if self.unit:checkPixelCollision(other, cols) then
    self.unit.root:hit(1)
	end
end

function C:onAppeared()
	print("Appeared...")
  game.animateCameraSpeed(0, 0.1)
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end