local C = {}

function C:init()
  self.c = 0
end

function C:onUpdate()
end

local offs = 0.1
local loffs = 0
local step = 80
local y = 20

function C:onAfterRenderUnit(unit)
  if unit.name ~= "bg" then return end
  self.palette:setColor(253, Color(self.c, self.c, self.c, self.c))
  self.c = self.c + game.deltaTime/3
  --pan.x = pan.x + math.sin(self.c*5)/2
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

    local pr0 = projectPoint(p0.x, p0.y, p0.z)
    local pr1 = projectPoint(p1.x, p1.y, p1.z)
    local pr2 = projectPoint(p2.x, p2.y, p2.z)
    local pr3 = projectPoint(p3.x, p3.y, p3.z)
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

    local pr0 = projectPoint(p0.x, p0.y, p0.z)
    local pr1 = projectPoint(p1.x, p1.y, p1.z)
    local pr2 = projectPoint(p2.x, p2.y, p2.z)
    local pr3 = projectPoint(p3.x, p3.y, p3.z)
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

    local pr0 = projectPoint(p0.x, p0.y, p0.z, true)
    local pr1 = projectPoint(p1.x, p1.y, p1.z, true)
    local pr2 = projectPoint(p2.x, p2.y, p2.z, true)
    local pr3 = projectPoint(p3.x, p3.y, p3.z, true)
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
    local pr0 = projectPoint(p0.x, p0.y, p0.z, true)
    local pr1 = projectPoint(p1.x, p1.y, p1.z, true)
    local pr2 = projectPoint(p2.x, p2.y, p2.z, true)
    local pr3 = projectPoint(p3.x, p3.y, p3.z, true)
    pf0 = Vec2(pr1.x+10, pr1.y)
    pf1 = Vec2(pr1.x - 70, pr1.y)
    pf2 = Vec2(pr1.x - 70, pr2.y)
    pf3 = Vec2(pr2.x+10, pr2.y)
    gfx:drawSpriteCustomQuad(self.iceSpr, pf0, pf1, pf2, pf3, 0, 0)
  end

  setProjectionPan(Vec2(game.cameraState.position.x + game.cameraState.positionOffset.x, game.cameraState.positionOffset.y))
  offs = offs + game.deltaTime * game.cameraState.speed
end

function C:onActivate()
  self.titleSpr = game:loadSprite("sprites/title")
  self.rocksSpr = game:loadSprite("sprites/rocks")
  self.lavaSpr = game:loadSprite("sprites/lava")
  self.iceSpr = game:loadSprite("sprites/ice")
  self.palette = gfx:createUserPalette()
  self.palette:copyFromSprite(self.titleSpr)
  --game:changeMusic("music/Retribution.ogg")
end

function C:onDeactivate()
  print("leave title screen")
end

function C:onSerialize(data)
  gfx:freeUserPalette(self.palette)
  data.c = self.c
  data.offs = offs
  data.loffs = loffs
  data.step = step
  data.y = y
end

function C:onDeserialize(data)
  dump(data)
  self.titleSpr = game:loadSprite("sprites/title")
  self.rocksSpr = game:loadSprite("sprites/rocks")
  self.lavaSpr = game:loadSprite("sprites/lava")
  self.iceSpr = game:loadSprite("sprites/ice")
  self.palette = gfx:createUserPalette()
  self.palette:copyFromSprite(self.titleSpr)
  self.c = data.c
  offs = data.offs
  loffs = data.loffs
  step = data.step
  y = data.y
end

return newInstance(C)
