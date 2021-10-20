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

-- Functions used for serialization
-- since we need to convert all C++ objects/refs to tables to be stored
-- during the serialization/deserialization

function unitFromId(id)
  return game:findUnitById(id)
end

function gameScreenFromId(id)
  return game:findGameScreenById(id)
end

function tableFromVec2(vec)
  return {x = vec.x, y = vec.y}
end

function tableToVec2(tbl)
  return Vec2(tbl.x, tbl.y)
end

function tableFromColor(col)
  return {r = col.r, g = col.g, b = col.b, a = col.a}
end

function tableToColor(tbl)
  return Color(tbl.r, tbl.g, tbl.b, tbl.a)
end

function tableFromRect(rc)
  return {x = rc.x, y = rc.y, width = rc.width, height = rc.height}
end

function tableToRect(tbl)
  return Rect(tbl.x, tbl.y, tbl.width, tbl.height)
end