local C = {}

local delay = 0

function C:init(gs)
  self.gameScreen = gs
  self.sprCopyright = game:loadSprite("sprites/copyright")
end

function C:onUpdate(deltaTime)
  delay = delay + deltaTime
  if delay > 0.5 then
    game:setScreenActive(self.gameScreen.name, false)
    game:changeMap(0)
    game:setScreenActive("play", true)
    game:setScreenActive("title", true)
    game:setScreenActive("credits", true)
    game:createPlayers()
  end
end

function C:onRender()
  gfx:drawSprite(self.sprCopyright, Rect(0, 0, 240, 320), 0, 0)
end

function C:onActivate()
end

function C:onDeactivate()
end

return newInstance(C)