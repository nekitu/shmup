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