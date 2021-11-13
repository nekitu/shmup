local C = {}

function C:init(unit)
  self.unit = unit
end

function C:onAppeared()
  print("apeared")
  game:fadeScreen(Color(1,0,0,0), Color(1,0,0,1), 5, true, 5)
end

return newInstance(C)
