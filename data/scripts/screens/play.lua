local C = {}

function C:init()
  self.fnt = game:loadFont("fonts/default")
  self.fntNum = game:loadFont("fonts/default_numeric")
  self.time = 0
  self.fpsCount = 0
  --game:fadeScreen(Color(0,0,0,0), Color(1,1,1,1), 4, true, 1)
end

function C:onUpdate()
end

function C:onRender()
  game:renderUnits()

  gfx.colorMode = ColorMode_Mul
  gfx.color = Color.red:getRgba()
  gfx:drawText(self.fnt, Vec2(90, 3), "HISCORE")
  gfx.colorMode = ColorMode_Add
  gfx.color = Color.green:getRgba()
  gfx:drawText(self.fnt, Vec2(90, 12), "OOOOOOO")

  local mp = game.mousePosition

  mp.x = math.ceil(mp.x)
  mp.y = math.ceil(mp.y)
  gfx:drawText(self.fnt, mp, "AICI!")

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

  gfx.colorMode = ColorMode_Add
  gfx.color = Color.black:getRgba()
  gfx:drawText(self.fnt, Vec2(20, 300), "FPS")
  gfx:drawText(self.fntNum, Vec2(50, 300), tostring(self.fps))
  gfx:drawText(self.fnt, Vec2(20, 290), "PCN")
  gfx:drawText(self.fntNum, Vec2(50, 290), tostring(game:getProjectileCount()))

  self.fpsCount = self.fpsCount + 1
  self.time = self.time + game.deltaTime
  if self.time > 1 then self.time = 0 self.fps = self.fpsCount self.fpsCount = 0 end
end

function C:onActivate()
  game:showMousePointer(false)
  game:setMusicVolume(0.0)
  game:setChannelVolume(SoundChannel_Enemy, 0.0)
  game:setChannelVolume(SoundChannel_Player, 0.0)
end

function C:onDeactivate()
end

return newInstance(C)