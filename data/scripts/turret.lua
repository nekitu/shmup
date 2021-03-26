local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
    local w = self.unit:findWeapon("gun1")

    if self.unit.appeared then
        w.active = true
    end

    self.unit.root:setFrameAnimationFromAngle(w.angle)

    if self.unit.health == 0 then
        self.unit.deleteMeNow = true
        local unit = game:spawn("units/turret_expl", "expl2", self.unit.root.position)
        unit.layerIndex = self.unit.layerIndex
        game:shakeCamera(Vec2(2, 2), 0.5, 70)
        game:fadeScreen(Color(1,1,1,0), Color(1,1,1,1), 0.1, true, 1)
    end
end

function C:onCollide(other, colPairs)
  self.unit.root:hit(1)
end

return newInstance(C)
