local C = {}

function C:init()
end

function C:onUpdate()
  game:changeMainScript("scripts/ingame_screen")
end

function C:onRender()
end

function C:onScreenEnter()
  print("enter title screen")
end

function C:onScreenLeave()
  print("leave title screen")
end

return function(unit)
  local o = {}
  setmetatable(o, C)
  C.__index = C
  o:init(unit)
  return o
end