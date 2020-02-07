local M = {}

local function onUpdate(unit)
  if unit.age > 0.5 then 
    unit.deleteMeNow = true
  end
end

local function onCollide(unitInst1, unitInst2)
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M