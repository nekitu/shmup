local M = {}

local function onUpdate(unit)
end

local function onCollide(unitInst1, unitInst2)
if true then return end
  local colCenter = Vec2(0, 0)
  if unitInst1.unit.type == 4 and unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance, colCenter) then
    unitInst1.deleteMeNow = true
  	if unitInst2.unit.type == 5 then unitInst2.deleteMeNow = true end
	local uinst = game.spawn("units/small_spark", "expl", colCenter)
	unitInst2.rootSpriteInstance:hit(5)
  end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M
