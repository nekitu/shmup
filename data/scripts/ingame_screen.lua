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
    gfx.currentColorMode = ColorMode_Mul
    gfx.currentColor = Color.red:getRgba()
    gfx:drawText(self.fnt, Vec2(90, 2), "HISCORE")
    gfx.currentColorMode = ColorMode_Add
    gfx.currentColor = Color.green:getRgba()
    gfx:drawText(self.fnt, Vec2(90, 12), "OOOOOOO")

    if self.time > 0.2 and self.time < 1 then
      gfx.currentColorMode = ColorMode_Mul
      gfx.currentColor = Color.red:getRgba()
      gfx:drawText(self.fnt, Vec2(8, 17), "   PLEASE")
      gfx.currentColorMode = ColorMode_Add
      gfx.currentColor = Color.black:getRgba()
      gfx:drawText(self.fnt, Vec2(10, 25), "INSERT COIN")

      gfx.currentColorMode = ColorMode_Add
      gfx.currentColor = Color.sky:getRgba()
      gfx:drawText(self.fnt, Vec2(135, 17), "   PLEASE")
      gfx.currentColorMode = ColorMode_Add
      gfx.currentColor = Color.black:getRgba()
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