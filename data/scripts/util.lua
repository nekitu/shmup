function dump(o)
  if type(o) == 'table' then
    local s = '{ '
    for k,v in pairs(o) do
    if type(k) ~= 'number' then k = '"'..k..'"' end
    s = s .. '['..k..'] = ' .. dump(v) .. ','
    end
    return s .. '} '
  else
    return tostring(o)
  end
end

function newInstance(C)
  return function(unit)
    local o = {}
    setmetatable(o, C)
    C.__index = C
    o:init(unit)
    return o
  end
end