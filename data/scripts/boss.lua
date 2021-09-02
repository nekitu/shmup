local C = {}

function C:init(unit)
  self.unit = unit
end

local from = Vec2(0,0)
local target = Vec2(40, -220)

local targets = { Vec2(40, -220), Vec2(-100, -400), Vec2(-100, -600), Vec2(120, -1000) }
local tindex = 1

local tlfrom = Vec2(0,0)
local tltarget = Vec2(0, 0)
local brfrom = Vec2(0,0)
local brtarget = Vec2(0, 0)
local trfrom = Vec2(0,0)
local trtarget = Vec2(0, 0)
local blfrom = Vec2(0,0)
local bltarget = Vec2(0, 0)

local tlleg
local trleg
local blleg
local brleg

local sprleg = game:loadSprite("sprites/leg")

function C:onSerialize(data)
  data.test1 = 123
end

function C:onDeserialize(data)
  dump(data)
  print("Deserialized boss: ", data.test1)
end

function C:onStageChange(old, new)
  print("CHANGE STAGE '" .. old .. "' to '" .. new .. "'")
  if not self.unit.appeared then return end
  if new == "stage0" then
    print("ACTIVATING STAGE0 "..tostring(self).. " " ..tostring(self.unit))
    local wp = self.unit:findWeapon("gun1")
    if wp then
      wp.active = true
    end
  end
  if new == "stage1" then
    print("ACTIVATING STAGE1 "..tostring(self).. " " ..tostring(self.unit))
    game:animateCameraSpeed(20, 0.6)
    self.unit:setAnimation("stage0_intro")
    self.unit:findWeapon("gun2").active = true
    self.unit:findWeapon("gun1").active = false
    print("ACTIVATING LAST RESORT SWORD")

    tlleg = self.unit:findSprite("tl_leg")
    trleg = self.unit:findSprite("tr_leg")
    blleg = self.unit:findSprite("bl_leg")
    brleg = self.unit:findSprite("br_leg")


    tlfrom.x = tlleg.position.x
    tlfrom.y = tlleg.position.y

    trfrom.x = trleg.position.x
    trfrom.y = trleg.position.y

    blfrom.x = blleg.position.x
    blfrom.y = blleg.position.y

    brfrom.x = brleg.position.x
    brfrom.y = brleg.position.y

    local tpos = self.unit:findSprite("movebody").position

    tltarget:set(tpos.x - 20, tpos.y - 40)
    trtarget:set(tpos.x + 20, tpos.y - 40)
    bltarget:set(tpos.x - 20, tpos.y + 20)
    brtarget:set(tpos.x + 20, tpos.y + 20)
  end
end

local s = 0
local tbody = 0
local t = 0
local legset = 0

local pauseTimer = 0

function C:onUpdate()
  if self.unit.health == 0 and self.unit:findSprite("movebody").visible then
    print("BOSS DESTROYED")
    game:animateTimeScale(0.1, 3, 1, 1)
    --self.unit.deleteMeNow = true
    local movebody = self.unit:findSprite("movebody")
    local loc = Vec2(self.unit.root.position.x + movebody.position.x, self.unit.root.position.y + movebody.position.y)
    local unit = game:spawn("units/turret_expl", "boss_expl2", loc)
    unit.layerIndex = self.unit.layerIndex
    --unit.root.collide = false
    game:animateCameraSpeed(30, 0.6)
    game:shakeCamera(Vec2(10, 10), 3, 200)
    self.unit:hideAllSprites()
    self.unit:disableAllWeapons()
    self.unit:findSprite("crater").visible = true
    self.unit:findSprite("crater").position:set(movebody.position.x, movebody.position.y)
    self.unit:playSound("explosion")
    game:changeMusic("music/gui.wav")
  end
  if self.unit.stageIndex == 1 then
    local movebody = self.unit:findSprite("movebody")
    movebody.position = from:lerp(targets[tindex], tbody)
    local legScale = 1 --0.5 + (1 - math.abs(t - 0.5) * 2) * 0.06

    if legset == 0 then
        tlleg.scale.x = legScale
        tlleg.scale.y = legScale
        brleg.scale.x = legScale
        brleg.scale.y = legScale
        tlleg.position = tlfrom:lerp(tltarget, t)
        brleg.position = brfrom:lerp(brtarget, t)
    else
        trleg.scale.x = legScale
        trleg.scale.y = legScale
        blleg.scale.x = legScale
        blleg.scale.y = legScale
        trleg.position = trfrom:lerp(trtarget, t)
        blleg.position = blfrom:lerp(bltarget, t)
    end

    local speed = 1.5

    if pauseTimer <= 0 then
    tbody = tbody + game.deltaTime * 0.1 * speed
    t = t + game.deltaTime * 1.5 * speed
    end

    pauseTimer = pauseTimer - game.deltaTime * 1.8 * speed

      tltarget:set(movebody.position.x - 20, movebody.position.y - 40)
      trtarget:set(movebody.position.x + 20, movebody.position.y - 40)
      bltarget:set(movebody.position.x - 20, movebody.position.y + 20)
      brtarget:set(movebody.position.x + 20, movebody.position.y + 20)

    if t > 1 then
      t = 0
      pauseTimer = 1
      legset = 1 - legset
      tlfrom.x = tlleg.position.x
      tlfrom.y = tlleg.position.y
      trfrom.x = trleg.position.x
      trfrom.y = trleg.position.y
      blfrom.x = blleg.position.x
      blfrom.y = blleg.position.y
      brfrom.x = brleg.position.x
      brfrom.y = brleg.position.y
    end
    if tbody > 1 then
      tbody = 0
      tindex = tindex + 1
      if tindex > #targets then tindex = #targets end
      from = Vec2(movebody.position.x, movebody.position.y) end
  end
end

function C:onBeforeRender()
  tlleg = self.unit:findSprite("tl_leg")
  trleg = self.unit:findSprite("tr_leg")
  blleg = self.unit:findSprite("bl_leg")
  brleg = self.unit:findSprite("br_leg")

  local movebody = self.unit:findSprite("movebody")

  local vtl = Vec2(tlleg.localRect:center().x - movebody.localRect:center().x, tlleg.localRect:center().y - movebody.localRect:center().y)
  local vtr = Vec2(trleg.localRect:center().x - movebody.localRect:center().x, trleg.localRect:center().y - movebody.localRect:center().y)
  local vbl = Vec2(blleg.localRect:center().x - movebody.localRect:center().x, blleg.localRect:center().y - movebody.localRect:center().y)
  local vbr = Vec2(brleg.localRect:center().x - movebody.localRect:center().x, brleg.localRect:center().y - movebody.localRect:center().y)

  vtl:normalize()
  vtr:normalize()
  vbr:normalize()
  vbl:normalize()

  local atl = vtl:dir2deg()
  local atr = vtr:dir2deg()
  local abl = vbl:dir2deg()
  local abr = vbr:dir2deg()

  local ptl = Vec2(movebody.localRect:center().x + (tlleg.localRect:center().x - movebody.localRect:center().x)/2, movebody.localRect:center().y + (tlleg.localRect:center().y - movebody.localRect:center().y)/2)
  local ptr = Vec2(movebody.localRect:center().x + (trleg.localRect:center().x - movebody.localRect:center().x)/2, movebody.localRect:center().y + (trleg.localRect:center().y - movebody.localRect:center().y)/2)
  local pbl = Vec2(movebody.localRect:center().x + (blleg.localRect:center().x - movebody.localRect:center().x)/2, movebody.localRect:center().y + (blleg.localRect:center().y - movebody.localRect:center().y)/2)
  local pbr = Vec2(movebody.localRect:center().x + (brleg.localRect:center().x - movebody.localRect:center().x)/2, movebody.localRect:center().y + (brleg.localRect:center().y - movebody.localRect:center().y)/2)

  ptl = game:worldToScreen(ptl, self.unit.layerIndex)
  ptr = game:worldToScreen(ptr, self.unit.layerIndex)
  pbl = game:worldToScreen(pbl, self.unit.layerIndex)
  pbr = game:worldToScreen(pbr, self.unit.layerIndex)

  gfx:drawSprite(sprleg, Rect(ptl.x - 6, ptl.y - 16, 12, 32), 0, -atl)
  gfx:drawSprite(sprleg, Rect(ptr.x - 6, ptr.y - 16, 12, 32), 0, -atr)
  gfx:drawSprite(sprleg, Rect(pbl.x - 6, pbl.y - 16, 12, 32), 0, -abl)
  gfx:drawSprite(sprleg, Rect(pbr.x - 6, pbr.y - 16, 12, 32), 0, -abr)
end

function C:onCollide(other, cols)
  local pos = Vec2(0, 0)
  --print(self.unit.health)
  for _,col in ipairs(cols) do
    col.sprite1:hit(15)
    col.sprite2:hit(15)
    print(col.sprite1.name, col.sprite1.health, col.sprite2.name, col.sprite2.health)
    if (col.sprite1.name == "lwing" and col.sprite1.health == 0) or (col.sprite2.name == "lwing" and col.sprite2.health == 0) then
      self.unit:replaceSprite("lwing", "lwing_damaged")
    end
    if (col.sprite1.name == "rwing" and col.sprite1.health == 0) or (col.sprite2.name == "rwing" and col.sprite2.health == 0) then
      self.unit:replaceSprite("rwing", "rwing_damaged")
    end
  end
end

function C:onAppeared()
  game:animateCameraSpeed(0, 0.1)
  local wp = self.unit:findWeapon("gun1")
  if wp then
    wp.active = true
  end
end

function C:onSerialize(data)
  data.coco = 111
  data.gogog = "AKAKALALA"
end

function C:onDeserialize(data)
  dump(data)
end

return newInstance(C)
