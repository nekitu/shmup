require("pickle")

function dumps(o)
  if type(o) == 'table' then
    local s = '{ '
    for k,v in pairs(o) do
    if type(k) ~= 'number' then k = '"'..k..'"' end
    s = s .. '['..k..'] = ' .. dumps(v) .. ','
    end
    return s .. '} '
  else
    return tostring(o)
  end
end

function dump(o)
  print(dumps(o))
end

function newInstance(C)
  return function(obj)
    local o = {}
    setmetatable(o, C)
    C.__index = C
    o:init(obj)
    return o
  end
end

local zoom = 1
local center = Vec2(120, 150)
local pan = Vec2(0, 0)

function setProjectionZoom(newZoom)
  zoom = newZoom
end

function setProjectionCenter(newCenter)
  center = newCenter
end

function setProjectionPan(newPan)
  pan = newPan
end

function projectPoint(x, y, z, roundCoords)
  local screen = Vec2()

  if z > 0 then
      screen.x = (x + pan.x) / z * zoom + center.x
      screen.y = (y + pan.y) / z * zoom + center.y
  end

  if roundCoords then
    screen.x = math.ceil(screen.x)
    screen.y = math.ceil(screen.y)
  end

  return screen
end
