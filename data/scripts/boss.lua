local M = {}

local function onUpdate(unitInst)
  --print("Updating "..unitInst.name.."  a:"..tostring(unitInst.rootSpriteInstance.transform.scale))
  --unitInst.rootSpriteInstance.transform.rotation = unitInst.rootSpriteInstance.transform.rotation + 1
  --unitInst.rootSpriteInstance.transform.scale = 5
  unitInst:fire()
end

local function onCollide(unitInst1, unitInst2)
  --print("Collision! "..unitInst1.name .. " "..unitInst2.name)
  if unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance) then
    unitInst1.rootSpriteInstance:hit(1)
  end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M