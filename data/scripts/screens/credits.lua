local C = {}

local credits = {
  {
    department = "CODING",
    people = {"NICUSOR NEDELCU", "JOHN DOE", "SUPERMAN"}
  },
  {
    department = "OBJECT ART",
    people = {"NICUSOR NEDELCU", "JOHN DOE", "SUPERMAN"}
  },
  {
    department = "SOUNDS",
    people = {"NICUSOR NEDELCU", "JOHN DOE", "SUPERMAN"}
  }
}

function C:init(gs)
  self.gameScreen = gs
  self:loadFonts()
  self.offset = gfx.videoHeight
end

function C:loadFonts()
  self.fntTitle = game:loadFont("fonts/credits_title")
  self.fntNormal = game:loadFont("fonts/default")
end

function C:onUpdate(dt)
  self.offset = self.offset - dt * 20
end

function C:onRender()
  gfx.colorMode = ColorMode_Add
  gfx.color = 0
  local y = self.offset
  for _, v in ipairs(credits) do
    local tsize = gfx:getTextSize(self.fntTitle, v.department)
    local pos = Vec2((gfx.videoWidth - tsize.x)/2, y)
    gfx:drawText(self.fntTitle, pos, v.department)
    y = y + tsize.y + 20

    for _, name in ipairs(v.people) do
      tsize = gfx:getTextSize(self.fntNormal, name)
      pos = Vec2((gfx.videoWidth - tsize.x)/2, y)
      gfx:drawText(self.fntNormal, pos, name)
      y = y + tsize.y + 10
    end
  end
end

function C:onActivate()
end

function C:onDeactivate()
end

function C:onSerialize(data)
  data.gameScreenId = self.gameScreen.id
  data.offset = self.offset
end

function C:onDeserialize(data)
  self.gameScreen = gameScreenFromId(data.gameScreenId)
  self:loadFonts()
  self.offset = data.offset
end

return newInstance(C)