local C = {}

function C:init(unit)
  self.unit = unit
	self.follower = nil
	self.follow = nil
	self.offset = Vec2()
	self.speed = 1
	self.constantSpeed = false
	self.offsetAcquired = false
end

function C:acquireOffset()
  if not self.unit.root then return end
  if not self.follow then self.follow = self.unit.root end

  if self.follower and self.follow then
    self.follower.notRelativeToRoot = true  -- force to not transform locally to parent
    self.offset = self.follower.position
    self.follower.position = self.follow.position + self.offset
    self.offsetAcquired = true
  end
end

function C:update()
  if not self.offsetAcquired then self:acquireOffset() end

  if self.follow and self.follower then
    local targetPos = self.follow.position + self.offset

    if self.constantSpeed then
      local delta = targetPos - self.follower.position
      local dist = delta:getLength()

      if dist <= self.speed * game.deltaTime then
        self.follower.position = targetPos
      else
        delta:normalize()
        self.follower.position:add(self.delta).addScalar(self.speed * game.deltaTime)
      end
    else
      self.follower.position:add((targetPos - self.follower.position):mulScalar(self.speed * game.deltaTime))
    end
  end
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end