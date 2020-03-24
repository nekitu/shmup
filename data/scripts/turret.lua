local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
    local w = self.unit:findWeapon("gun1")

    if self.unit.appeared then
        w.active = true
    end

    self.unit.root:setFrameAnimationFromAngle(w.angle - 180)
    self.unit.root.frame = self.unit.root:getFrameFromAngle(w.angle - 180)

    if self.unit.health == 0 then
        self.unit.deleteMeNow = true
        local uinst = game:spawn("units/turret_expl", "expl2", self.unit.root.position)
        uinst.layerIndex = self.unit.layerIndex
        game:shakeCamera(Vec2(2, 2), 0.5, 70)
        game:fadeScreen(Color(1,1,1,1), 0, 0.1, true)
    end
end

function C:onCollide(other)
  local pos = Vec2(0, 0)
  if self.unit.root:checkPixelCollision(other.root, pos) then
    self.unit.root:hit(1)
  end
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end