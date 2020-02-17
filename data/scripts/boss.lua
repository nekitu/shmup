local M = {}

local function onUpdate(unitInst)
  --print("Updating "..unitInst.name.."  a:"..tostring(unitInst.rootSpriteInstance.transform.scale))
  --unitInst.rootSpriteInstance.transform.rotation = unitInst.rootSpriteInstance.transform.rotation + 1
  --unitInst.rootSpriteInstance.transform.scale = 5
  unitInst:fire()
end

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

local function onCollide(unitInst1, unitInst2)
  --print("Collision! "..unitInst1.name .. " "..unitInst2.name)
  local pos = Vec2(0, 0)
  if unitInst1.rootSpriteInstance:checkPixelCollision(unitInst2.rootSpriteInstance, pos) then
  end
  local cols={}
  if unitInst1:checkPixelCollision(unitInst2, cols) then
  	--print("COLS "..tostring(#cols)) dump(cols)
  unitInst1.rootSpriteInstance:hit(1)

	end
end

M.onCollide = onCollide
M.onUpdate = onUpdate

return M
