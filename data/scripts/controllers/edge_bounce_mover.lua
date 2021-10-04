local C = {}

function C:init(unit)
  self.unit = unit
end

function C:setup(params)
  self.direction = Vec2(randomFloat(-1,1), randomFloat(-1,1)):getNormalized()
end

function C:onUpdate()
  if not self.unit.root then return end

  if self.unit.root.rect.x + self.unit.root.rect.width > gfx.videoWidth then
    self.direction.x = -math.abs(self.direction.x)
  end
  if self.unit.root.rect.x < 0 then
    self.direction.x = math.abs(self.direction.x)
  end

  if self.unit.root.rect.y + self.unit.root.rect.height > gfx.videoHeight then
    self.direction.y = -math.abs(self.direction.y)
  end
  if self.unit.root.rect.y < 0 then
    self.direction.y = math.abs(self.direction.y)
  end

  self.unit.root.position.x = self.unit.root.position.x + self.unit.speed * game.deltaTime * self.direction.x
  self.unit.root.position.y = self.unit.root.position.y + self.unit.speed * game.deltaTime * self.direction.y
end

return newInstance(C)