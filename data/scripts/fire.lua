local M = {}

engine.log("COCOCANEL Init")

local function onUpdate(nr, str)
  engine.log(tostring(nr)..str)
end

M.onUpdate = onUpdate

return M