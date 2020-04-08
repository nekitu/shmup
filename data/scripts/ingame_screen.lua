local C = {}

function C:init()
  self.fnt = game:loadFont("fonts/default")
  self.time = 0
end

function C:onUpdate()
end

function C:onRender(layerIndex)
  --print("Rendering "..tostring(layerIndex))
  if layerIndex == 0 then
    gfx.colorMode = ColorMode_Mul
    gfx.color = Color.red:getRgba()
    gfx:drawText(self.fnt, Vec2(90, 2), "HISCORE")
    gfx.colorMode = ColorMode_Add
    gfx.color = Color.green:getRgba()
    gfx:drawText(self.fnt, Vec2(90, 12), "OOOOOOO")

    if self.time > 0.2 and self.time < 1 then
      gfx.colorMode = ColorMode_Mul
      gfx.color = Color.red:getRgba()
      gfx:drawText(self.fnt, Vec2(8, 17), "   PLEASE")
      gfx.colorMode = ColorMode_Add
      gfx.color = Color.black:getRgba()
      gfx:drawText(self.fnt, Vec2(10, 25), "INSERT COIN")

      gfx.colorMode = ColorMode_Add
      gfx.color = Color.sky:getRgba()
      gfx:drawText(self.fnt, Vec2(135, 17), "   PLEASE")
      gfx.colorMode = ColorMode_Add
      gfx.color = Color.black:getRgba()
      gfx:drawText(self.fnt, Vec2(140, 25), "INSERT COIN")
    end

    self.time = self.time + game.deltaTime
    if self.time > 1 then self.time = 0 end
  end
end

function C:onScreenEnter()
end

function C:onScreenLeave()
end

return function()
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init()
  return o
end