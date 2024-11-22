require "class"

Vector2 = Class(function(self, x, y)
    self.x, self.y = x or 0, y or 0
end)