local C = {}

function C:init()
end

function C:onUpdate()
  
  --game:changeMainScript("scripts/ingame_screen")
end

function C:onRender()
  gfx:drawSprite(self.titleSpr, Rect(gfx.videoWidth / 2 - 150/2, 0, 150, 150), 0, 0)
end

function C:onScreenEnter()
  print("enter title screen")
  self.titleSpr = game:loadSprite("sprites/title")
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