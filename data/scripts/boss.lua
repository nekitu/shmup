local M = {}

local function onStageChange(unitInst, old, new)
	print("Change stage to "..new .. " from " .. old)
	if new == "stage0" then
		print("ACTIVATING MAGEBALAST")
		unitInst:findWeapon("gun1").active = true
	end
	if new == "stage1" then
		unitInst:findWeapon("gun2").active = true

	print("ACTIVATING LAST RESORT SWORD") end
end

local function onStageUpdate(unitInst, name)
	--print("Updating stage " .. name .. " health "..tostring(unitInst.health))
end

local function onUpdate(unitInst)
	if unitInst.health == 0 then
        unitInst.deleteMeNow = true
        local uinst = game.spawn("units/turret_expl", "expl2", unitInst.rootSpriteInstance.transform.position)
        uinst.layerIndex = unitInst.layerIndex
		game.animateCameraSpeed(110, 0.6)
		game.shakeCamera(game.player1, "screenFx", Vec2(10, 10), 3, 200)
	end
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

local function onAppeared(unitInst)
	print("Appeared...")
	game.animateCameraSpeed(0, 0.1)
end

M.onCollide = onCollide
M.onUpdate = onUpdate
M.onStageChange = onStageChange
M.onAppeared = onAppeared

return M
