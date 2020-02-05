local M = {}
local projData = {}

local function onUpdate(unit)
end

local function onCollide(unitInst1, unitInst2)
  print("Collider projectile "..tostring(unitInst1.unit.type) .. " " .. tostring(unitInst2.unit.type))
  if projData[unitInst1.name] == nil then print("cret "..tostring(unitInst1.name)) projData[unitInst1.name] = {hits = 0} end
  projData[unitInst1.name].hits = projData[unitInst1.name].hits + 1
  if unitInst1.unit.type == 5 then unitInst1.deleteMeNow = true end
  print(projData[unitInst1.name].hits)
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M