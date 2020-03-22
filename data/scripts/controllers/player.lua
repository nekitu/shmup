local C = {}

function C:init(unit)
  self.unit = unit
  self.playerIndex = 0
  self.isFirePressed = false
  self.active = true
end

function C:update()
  if not self.active then
    return
  end

  if not self.unit.root then return end
  self.unit.root.position.y = self.unit.root.position.y - game.cameraSpeed * game.deltaTime

  if game:isPlayerFire1(self.playerIndex) and not self.isFirePressed then
    self.isFirePressed = true
    for _, weapon in ipairs(self.unit:getWeapons()) do
      weapon:fire()
    end
  end

  if game:isPlayerFire1(self.playerIndex) then
    self.isFirePressed = false
  end

  if self.isFirePressed then
    for _, weapon in ipairs(self.unit:getWeapons()) do
      weapon.active = true
    end
  else
    for _, weapon in ipairs(self.unit:getWeapons()) do
      weapon.active = false
    end
  end

  local moveDir = Vec2(0,0)
  if game:isPlayerMoveLeft(self.playerIndex) then
    moveDir.x = -1
  end
  if game:isPlayerMoveRight(self.playerIndex) then
    moveDir.x = 1
  end
  if game:isPlayerMoveUp(self.playerIndex) then
    moveDir.y = -1
  end
  if game:isPlayerMoveDown(self.playerIndex) then
    moveDir.y = 1
  end
  moveDir:normalize()
  self.unit.root.position:add(moveDir:mulScalarReturn(game.deltaTime * self.unit.speed))
  util.clampValue(self.unit.root.position.x, game.cameraParallaxOffset * (-1), gfx.videoWidth - game.cameraParallaxOffset)
  util.clampValue(self.unit.root.position.y, game.cameraPosition.y * (-1), gfx.videoHeight - game.cameraPosition.y)
  game.cameraParallaxOffset = (gfx.videoWidth / 2 - self.unit.root.position.x) * game.cameraParallaxScale

end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end