local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onUpdate()
end

function C:onAppeared()
  self.unit:setAnimation("intro1")
end

function C:onAnimationEvent(sprite, eventName)
  print("Animation event: " .. eventName)
end

return newInstance(C)