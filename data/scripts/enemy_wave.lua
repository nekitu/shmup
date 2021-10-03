local C = {}

function C:init(unit)
  self.unit = unit
  self.activeTime = unit:getFloat("activeTime", 10)
  self.spawnDelay = unit:getFloat("spawnDelay", 2)
  self.mover = unit:getString("mover", "")
  self.spawnUnit = unit:getString("spawnUnit", "")
  self.spawnTimer = 0
  self.activeTimer = 0
end

function C:onUpdate()
  self.activeTimer = self.activeTimer + game.deltaTime
  self.spawnTimer = self.spawnTimer + game.deltaTime
  if self.activeTimer >= self.activeTime then self.unit.deleteMeNow = true end

  if self.spawnTimer >= self.spawnDelay then
    local unit = game:spawn(self.spawnUnit, "SpawnedWaveUnit", self.unit.root.position)
    self.spawnTimer = 0
    unit:addController(self.mover, { lineDirection = Vec2(1, 1) })
  end
end

function C:onCollide(other, colPairs)
end

return newInstance(C)
