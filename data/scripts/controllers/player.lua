local C = {}

function C:init(unit)
  self.unit = unit
  self.playerControl = true
end

function C:setup(params)
  self.playerIndex = params:getInt("playerIndex", 0)
  self.active = params:getBool("active", true)
  log.info("Setup player " .. tostring(self.playerIndex))
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
    -- fire if the button is pressed, the repeat fire logic is done in the engine
    -- TODO: fire1 and fire2 handle, not all weapons
    if game:isPlayerFire1(self.playerIndex) then
      for _, weapon in ipairs(self.unit:getWeapons()) do
        weapon:fire()
      end
    else
      for _, weapon in ipairs(self.unit:getWeapons()) do
        weapon:stopFire()
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
  self.unit.root.position:add(moveDir:mulScalarReturn(game.realDeltaTime * self.unit.speed))
  self.unit.root.position.x = util.clampValue(self.unit.root.position.x, 0, gfx.videoWidth)
  self.unit.root.position.y = util.clampValue(self.unit.root.position.y, 0, gfx.videoHeight)
  game.cameraState.parallaxOffset = (gfx.videoWidth / 2 - self.unit.root.position.x) * game.cameraState.parallaxScale
  collectgarbage()
  collectgarbage("count")
end

return newInstance(C)