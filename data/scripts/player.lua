local M = {}

local function onUpdate(unit)
  --print("Updating "..unit.name)
end

local function onCollide(unitInst1, unitInst2)
  --print("Collision! "..unitInst1.name .. " "..unitInst2.name)
  --unitInst1.rootSpriteInstance:hit(1)
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M
