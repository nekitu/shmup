local M = {}

game.loadSprite("sprites/turret_expl")
game.loadSprite("sprites/big_expl")

local function onUpdate(unitInst)
  --print("Updating "..unitInst.name.."  a:"..tostring(unitInst.rootSpriteInstance.transform.scale))
  --unitInst.rootSpriteInstance.transform.rotation = unitInst.rootSpriteInstance.transform.rotation + 1
  --unitInst.rootSpriteInstance.transform.scale = 5
  --unitInst:fire()
    local w = unitInst:findWeapon("gun1")

    if unitInst.appeared then
        w.active = true
    end

    unitInst.rootSpriteInstance:setFrameAnimationFromAngle(w.angle - 180)

	--unitInst.rootSpriteInstance.animationFrame = unitInst.rootSpriteInstance:getFrameFromAngle(w.angle - 180)
    --print(unitInst.rootSpriteInstance.animationFrame)
    if unitInst.health == 0 then
        unitInst.deleteMeNow = true
        local uinst = game.spawn("units/turret_expl", "expl2", unitInst.rootSpriteInstance.transform.position)
        uinst.layerIndex = unitInst.layerIndex
        print("Spawned ".. uinst.unit.name .." at "..tostring(uinst.rootSpriteInstance.transform.position.x) .." "..tostring(uinst.rootSpriteInstance.transform.position.y))
    end
end

local function onCollide(unitInst1, unitInst2)
  --print("Collision! "..unitInst1.name .. " "..unitInst2.name)
  local pos = Vec2(0, 0)
  if unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance, pos) then
    unitInst1.rootSpriteInstance:hit(1)
  end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M
