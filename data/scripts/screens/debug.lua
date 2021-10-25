local C = {}
local offset = gfx.videoHeight

function C:init()
  self.fnt = game:loadFont("fonts/small8x8")
  self.active = false
end

function C:onUpdate(dt)
  if input:wasPressed("debug") then self.active = not self.active end
end

function C:onRender()
  if not self.active then return end
  gfx.colorMode = ColorMode_Add
  gfx.color = 0
  local count = game:getUnitCount()
  for i = 0, count - 1 do
    local unit = game:getUnit(i)
    --local pos = Vec2(unit.root.position.x, unit.root.position.y)
    gfx:drawRect(unit.boundingBox, 1)
    --gfx:drawText(self.fnt, pos, "UNIT:" .. unit.name)
  end
  gfx:drawLine(Vec2(120,160),
   Vec2(120 + input:getValue("axisleftx")*120, 160 + input:getValue("axislefty")*160), 1)
  --gfx:drawText(self.fnt, Vec2(0, 10), "AXIS:" .. tostring(input:getValue("axisleft")))
end

function C:onActivate()
  if not game.prebakedAtlas then
    util.saveAtlas("atlas")
  end
end

function C:onDeactivate()
end

return newInstance(C)