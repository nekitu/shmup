local M = {}

local function onUpdate(unit)
end

local function onCollide(unitInst1, unitInst2)
	if unitInst1.unit.type == 4 and unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance) then
	unitInst1.deleteMeNow = true
    local uinst = game.spawn("units/small_spark", "expl", unitInst1.rootSpriteInstance.transform.position)
    uinst.rootSpriteInstance.transform.scale = 1
  end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M
