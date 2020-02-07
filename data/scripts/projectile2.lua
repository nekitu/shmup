local M = {}

local function onUpdate(unit)
end

local function onCollide(unitInst1, unitInst2)
  if unitInst1.unit.type == 4 and unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance) then
    unitInst1.deleteMeNow = true
    local uinst = engine.spawnUnitInstance("units/small_spark", "expl", unitInst1.rootSpriteInstance.transform.position)
  end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M