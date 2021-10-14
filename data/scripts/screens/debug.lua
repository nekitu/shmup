local C = {}
local offset = gfx.videoHeight

function C:init()
  self.fnt = game:loadFont("fonts/small8x8")
end

function C:onUpdate(dt)
end

function C:onRender()
  gfx.colorMode = ColorMode_Add
  gfx.color = 0
  local count = game:getUnitCount()
  for i = 0, count - 1 do
    local unit = game:getUnit(i)
    local pos = Vec2(unit.root.position.x, unit.root.position.y)
    gfx:drawText(self.fnt, pos, "UNIT:" .. unit.name)
  end
  gfx:drawLine(Vec2(160,120), game.mousePosition, 1)
end

function C:onActivate()
end

function C:onDeactivate()
end

return newInstance(C)