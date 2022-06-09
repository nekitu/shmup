local C = {}

function C:init(unit)
  self.unit = unit
  
end

function C:onUpdate()
	local psys = game:findUnitByName("particles")
	
	if psys then
		psys.root.position = game:screenToWorld(self.unit.root.position, self.unit.layerIndex)
	end
end

return newInstance(C)