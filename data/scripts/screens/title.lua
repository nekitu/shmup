local C = {}

function C:init()
  self.c = 0
end

function C:onUpdate()

  --game:changeMainScript("scripts/ingame_screen")
end

function C:onRender()
  self.palette:setColor(253, Color(self.c, self.c, self.c, self.c))
  self.c = self.c + game.deltaTime/3
  --gfx:drawPalettedSprite(self.titleSpr, Rect(gfx.videoWidth / 2 - 150/2, 0, 150, 150), 0, 0, self.palette)
  gfx:drawPalettedSpriteCustomQuad(self.titleSpr, Vec2(10, 10),Vec2(100, 30),Vec2(100+self.c*10, 130+self.c*10),Vec2(10,130), 0, 0, self.palette)
end

function C:onScreenEnter()
  print("enter title screen")
  self.titleSpr = game:loadSprite("sprites/title")
  self.palette = gfx:createUserPalette()
  self.palette:copyFromSprite(self.titleSpr)
end

function C:onScreenLeave()
  print("leave title screen")
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end