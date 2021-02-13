local C = {}

function C:init(unit)
  self.unit = unit
  self.isFirePressed = false
  self.playerControl = true
end

function C:setup(params)
  self.playerIndex = params:getInt("playerIndex", 0)
  self.active = params:getBool("active", true)
  log.info("Setup player " .. tostring(self.playerIndex))

  self.playerControl = false
  print(self.unit)
  self.unit:setAnimation("player_intro")
end

function C:onAnimationEvent(sprite, eventName)
  if eventName == "endPlayerIntro" then
    print("==========PLAYER HAS CONTROL NOW")
    self.playerControl = true
  end
end

function C:onUpdate()
  if not self.active then
    return
  end

  if not self.unit.root then return end

  local moveDir = Vec2()

  if self.playerControl then
    if game:isPlayerFire1(self.playerIndex) and not self.isFirePressed then
      self.isFirePressed = true
      for _, weapon in ipairs(self.unit:getWeapons()) do
        weapon:fire()
      end
    end

    if not game:isPlayerFire1(self.playerIndex) then
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

    if game:isPlayerMoveLeft(self.playerIndex) then
      moveDir.x = -1
      self.unit.root:setFrameAnimation("left")
    end
    if game:isPlayerMoveRight(self.playerIndex) then
      moveDir.x = 1
      self.unit.root:setFrameAnimation("right")
    end
    if game:isPlayerMoveUp(self.playerIndex) then
      moveDir.y = -1
    end
    if game:isPlayerMoveDown(self.playerIndex) then
      moveDir.y = 1
    end
  end

  if self.unit.root:getFrameAnimationName() == "left" and not self.unit.root.animationIsActive then
    self.unit.root:setFrameAnimation("default")
  end

  if self.unit.root:getFrameAnimationName() == "right" and not self.unit.root.animationIsActive then
    self.unit.root:setFrameAnimation("default")
  end

  moveDir:normalize()
  self.unit.root.position:add(moveDir:mulScalarReturn(game.deltaTime * self.unit.speed))
  self.unit.root.position.x = util.clampValue(self.unit.root.position.x, 0, gfx.videoWidth)
  self.unit.root.position.y = util.clampValue(self.unit.root.position.y, 0, gfx.videoHeight)
  game.cameraParallaxOffset = (gfx.videoWidth / 2 - self.unit.root.position.x) * game.cameraParallaxScale
  collectgarbage()
  collectgarbage("count")
end

return newInstance(C)