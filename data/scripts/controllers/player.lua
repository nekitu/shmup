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
    if game:isPlayerFire1(self.playerIndex) then
      local wpns = self.unit:getGroupWeapons(0)
      for _, wpn in ipairs(wpns) do
        wpn:fire()
      end
    else
      local wpns = self.unit:getGroupWeapons(0)
      for _, wpn in ipairs(wpns) do
        wpn:stopFire()
      end
    end

    if game:isPlayerFire2(self.playerIndex) then
      local wpns = self.unit:getGroupWeapons(1)
      for _, wpn in ipairs(wpns) do
        wpn:fire()
      end
    else
      local wpns = self.unit:getGroupWeapons(1)
      for _, wpn in ipairs(wpns) do
        wpn:stopFire()
      end
    end

    if game:isPlayerFire3(self.playerIndex) then
      local bomb = self.unit:findSprite("bomb")
      if bomb then
        bomb.visible = true
        bomb:setFrameAnimation("explode")
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