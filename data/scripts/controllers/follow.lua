require("util")
local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
	self.follower = self.unit:findSprite(params:getString("follower", ""))
	self.follow = self.unit:findSprite(params:getString("follow", ""))
	self.offset = params:getVec2("offset", Vec2())
	self.speed = params:getFloat("speed", 1)
	self.constantSpeed = params:getBool("constantSpeed", false)
  self.offsetAcquired = false
end

function C:onUnload()
  print("Unloading script...")
  self.follower.relativeToRoot = self.initialRelativeToRoot
  self.follower.position = self.initialPosition:getCopy()
end

function C:acquireOffset()
  if not self.unit.root then return end
  if not self.follow then self.follow = self.unit.root end

  if self.follower and self.follow then
    self.initialRelativeToRoot = self.follower.relativeToRoot
    self.initialPosition = self.follower.position:getCopy()
    self.follower.relativeToRoot = false  -- no parent
    self.offset = self.follower.position:getCopy()
    self.follower.position = self.follow.position + self.offset
    self.offsetAcquired = true
  end
end

function C:onUpdate()
  if not self.offsetAcquired then self:acquireOffset() end

  if self.follow and self.follower then
    local targetPos = self.follow.position + self.offset

    if self.constantSpeed then
      local delta = targetPos - self.follower.position
      local dist = delta:getLength()
      local moveSize = self.speed * game.deltaTime

      if dist <= moveSize then
        self.follower.position = targetPos:getCopy()
      else
        delta:normalize()
        self.follower.position:add(delta:mulScalar(moveSize))
      end
    else
      local t = self.speed * game.deltaTime
      self.follower.position = self.follower.position + (targetPos - self.follower.position):mulScalar(t)
    end
  end
end

function C:onSerialize(data)
  data.unitId = self.unit.id
  data.followSprId = self.follow.id
  data.followerSprId = self.follower.id
  data.initialRelativeToRoot =  self.initialRelativeToRoot
  data.initialPosition = tableFromVec2(self.initialPosition)
  data.offset = tableFromVec2(self.offset)
  data.offsetAcquired = self.offsetAcquired
end

function C:onDeserialize(data)
  self.unit = unitFromId(data.unitId)
  self.follow = data.followSprId
  self.follower = data.followerSprId
  self.initialRelativeToRoot =  data.initialRelativeToRoot
  self.initialPosition = tableToVec2(data.initialPosition)
  self.offset = tableToVec2(data.offset)
  self.offsetAcquired = data.offsetAcquired
end

return newInstance(C)