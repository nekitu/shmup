local C = {}

local zoom = 1

local center = Vec2(120, 150)
local pan = Vec2(0, 0)

local function project(x, y, z, center)
  local screen = Vec2()
  if z > 0 then
      screen.x = (x + pan.x) / z * zoom + center.x
      screen.y = (y + pan.y) / z * zoom + center.y
  end
  return screen
end

function C:init()
  self.c = 0
end

function C:onUpdate()

  --game:changeMainScript("scripts/ingame_screen")
end
local offs = 0.1
local loffs = 0
local step = 80
local y = 20
function C:onRender()
  self.palette:setColor(253, Color(self.c, self.c, self.c, self.c))
  self.c = self.c + game.deltaTime/3
  pan.x = pan.x + math.sin(self.c*5)/2
  --gfx:drawPalettedSprite(self.titleSpr, Rect(gfx.videoWidth / 2 - 150/2, 0, 150, 150), 0, 0, self.palette)
  --gfx:drawPalettedSpriteCustomQuad(self.titleSpr, Vec2(10, 10),Vec2(100, 30),Vec2(100+self.c*10, 130+self.c*10),Vec2(10,130), 0, 0, self.palette)
  for i = 0, 20 do
    local p0 = {x = 70, y = -1500, z = 2}
    local p1 = {x = 70, y = -1500, z = 1}
    local p2 = {x = 70, y = -1500 + 80, z = 1}
    local p3 = {x = 70, y = -1500 + 80, z = 2}
    p0.y = p0.y + offs + step * i
    p1.y = p1.y + offs + step * i
    p2.y = p2.y + offs + step * i
    p3.y = p3.y + offs + step * i

    local pr0 = project(p0.x, p0.y, p0.z, center)
    local pr1 = project(p1.x, p1.y, p1.z, center)
    local pr2 = project(p2.x, p2.y, p2.z, center)
    local pr3 = project(p3.x, p3.y, p3.z, center)
    gfx:drawSpriteCustomQuad(self.rocksSpr, pr0, pr1, pr2, pr3, 0, 0)
  end

  for i = 0, 20 do
    local p0 = {x = -70, y = -1500, z = 2}
    local p1 = {x = -70, y = -1500, z = 1}
    local p2 = {x = -70, y = -1500 + 80, z = 1}
    local p3 = {x = -70, y = -1500 + 80, z = 2}
    p0.y = p0.y + offs + step * i
    p1.y = p1.y + offs + step * i
    p2.y = p2.y + offs + step * i
    p3.y = p3.y + offs + step * i

    local pr0 = project(p0.x, p0.y, p0.z, center)
    local pr1 = project(p1.x, p1.y, p1.z, center)
    local pr2 = project(p2.x, p2.y, p2.z, center)
    local pr3 = project(p3.x, p3.y, p3.z, center)
    gfx:drawSpriteCustomQuad(self.rocksSpr, pr0, pr1, pr2, pr3, 0, 0)
    local pf0 = Vec2(pr0.x, pr0.y)
    local pf1 = Vec2(pr0.x + 70, pr0.y)
    local pf2 = Vec2(pr3.x + 70, pr3.y)
    local pf3 = Vec2(pr3.x, pr3.y)
    gfx:drawSpriteCustomQuad(self.lavaSpr, pf0, pf1, pf2, pf3, 0, 0)
  end


  for i = 0, 20 do
    local p0 = {x = 70, y = -1500, z = 2}
    local p1 = {x = 70, y = -1500, z = 1}
    local p2 = {x = 70, y = -1500 + 80, z = 1}
    local p3 = {x = 70, y = -1500 + 80, z = 2}
    p0.y = p0.y + offs + step * i
    p1.y = p1.y + offs + step * i
    p2.y = p2.y + offs + step * i
    p3.y = p3.y + offs + step * i

    local pr0 = project(p0.x, p0.y, p0.z, center)
    local pr1 = project(p1.x, p1.y, p1.z, center)
    local pr2 = project(p2.x, p2.y, p2.z, center)
    local pr3 = project(p3.x, p3.y, p3.z, center)
    local pf0 = Vec2(pr1.x-10, pr1.y)
    local pf1 = Vec2(pr1.x + 70, pr1.y)
    local pf2 = Vec2(pr2.x + 70, pr2.y)
    local pf3 = Vec2(pr2.x -10, pr2.y)
    gfx:drawSpriteCustomQuad(self.iceSpr, pf0, pf1, pf2, pf3, 0, 0)
  end

  for i = 0, 20 do
    local p0 = {x = -70, y = -1500, z = 2}
    local p1 = {x = -70, y = -1500, z = 1}
    local p2 = {x = -70, y = -1500 + 80, z = 1}
    local p3 = {x = -70, y = -1500 + 80, z = 2}
    p0.y = p0.y + offs + step * i
    p1.y = p1.y + offs + step * i
    p2.y = p2.y + offs + step * i
    p3.y = p3.y + offs + step * i
    local pr0 = project(p0.x, p0.y, p0.z, center)
    local pr1 = project(p1.x, p1.y, p1.z, center)
    local pr2 = project(p2.x, p2.y, p2.z, center)
    local pr3 = project(p3.x, p3.y, p3.z, center)
    pf0 = Vec2(pr1.x+10, pr1.y)
    pf1 = Vec2(pr1.x - 70, pr1.y)
    pf2 = Vec2(pr1.x - 70, pr2.y)
    pf3 = Vec2(pr2.x+10, pr2.y)
    gfx:drawSpriteCustomQuad(self.iceSpr, pf0, pf1, pf2, pf3, 0, 0)
  end

  offs = offs + game.deltaTime * 55
end

function C:onScreenEnter()
  print("enter title screen")
  self.titleSpr = game:loadSprite("sprites/title")
  self.rocksSpr = game:loadSprite("sprites/rocks")
  self.lavaSpr = game:loadSprite("sprites/lava")
  self.iceSpr = game:loadSprite("sprites/ice")
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