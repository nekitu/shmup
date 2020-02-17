local M = {}
local fnt = engine.loadFont("fonts/default")
local xxx = 0.0
local t = 0
GLOGO = 1
local LOLO = 1

function M.onRender(layerIndex)
  gfx.drawText(fnt, Vec2(xxx, 150+math.sin(t*10)*10), "GAME OVER")
  xxx = 120 + math.sin(t)*122
  t = t + engine.deltaTime
end

function M.onUpdate()
	GLOGO = GLOGO + 1
	LOLO = LOLO + 1
	--print("GLOGO "..tostring(GLOGO))
	--print("LOLO "..tostring(LOLO))
end

return M
