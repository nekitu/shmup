local M = {}

local function onUpdate(unitInst)
end

local function onCollide(unitInst1, unitInst2)
end

local function onAppeared(unitInst)
  game.fadeScreen(game.player1, "screenFx", Color(0,0,0,1), game.ColorMode_Mul, 3, false)
end

M.onCollide = onCollide
M.onUpdate = onUpdate
M.onAppeared = onAppeared

return M