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
  self.actionPrefix = "player" .. tostring(self.playerIndex + 1) .. "_"
  self.unit.selected = true
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

  if input:wasPressed("coin") then
    print("COIN!")
  end

  if self.playerControl then
    -- fire if the button is pressed, the repeat fire logic is done in the engine
    if input:isDown(self.actionPrefix .. "fire1") or input:getValue(self.actionPrefix .. "fire1") > 0.5 then
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

    if input:isDown(self.actionPrefix .. "fire2") or input:getValue(self.actionPrefix .. "fire2") > 0.5 then
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

    if input:wasDown(self.actionPrefix .. "fire3") then
      local bomb = self.unit:findSprite("bomb")
      if bomb then
        bomb.visible = true
        bomb:setFrameAnimation("explode")
      end
    end
    if input:isDown(self.actionPrefix .. "moveLeft") or input:getValue(self.actionPrefix .. "moveLeft") < -0.3 then
      moveDir.x = -1
      self.unit.root:setFrameAnimation("left")
    end
    if input:isDown(self.actionPrefix .. "moveRight") or input:getValue(self.actionPrefix .. "moveRight") > 0.3  then
      moveDir.x = 1
      self.unit.root:setFrameAnimation("right")
    end
    if input:isDown(self.actionPrefix .. "moveUp") or input:getValue(self.actionPrefix .. "moveUp") < -0.3 then
      moveDir.y = -1
    end
    if input:isDown(self.actionPrefix .. "moveDown") or input:getValue(self.actionPrefix .. "moveDown") > 0.3 then
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

function C:onSerialize(data)
  data.unitId = self.unit.id
  data.playerIndex = self.playerIndex
  data.active = self.active
  data.actionPrefix = self.actionPrefix
end

function C:onDeserialize(data)
  self.unit = unitFromId(data.unitId)
  self.playerIndex = data.playerIndex
  self.active = data.active
  self.actionPrefix = data.actionPrefix
end

return newInstance(C)