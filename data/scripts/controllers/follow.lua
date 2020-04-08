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
  print(tostring(self.follower))
  print(tostring(self.follow))
  dump(self)
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

function C:onUpdate()
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
        self.follower.position:add(delta:mulScalar(self.speed * game.deltaTime))
      end
      print("dx "..tostring(delta.x))
      print("dy "..tostring(delta.y))
    else
      local v = targetPos:sub(self.follower.position)
      print("speed: "..tostring(self.speed))
      print("deltatime: "..tostring(game.deltaTime))
      print("m: "..tostring(self.speed * game.deltaTime))
      print(v.x)
      print(v.y)
      v:mulScalar(self.speed * game.deltaTime)
      print(v.x)
      print(v.y)
      self.follower.position:add(v)
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