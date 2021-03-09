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


local offset = gfx.videoHeight

function C:init()
  self.fntTitle = game:loadFont("fonts/credits_title")
  self.fntNormal = game:loadFont("fonts/default")
end

function C:onUpdate(dt)
  offset = offset - dt * 20
end

function C:onRender()
  gfx.colorMode = ColorMode_Add
  gfx.color = 0
  local y = offset
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

return newInstance(C)