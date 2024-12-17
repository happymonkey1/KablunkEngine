require "class"

Vector4 = Class(function(self, x, y, z, w)
    self.x, self.y, self.z, self.w = x or 0, y or 0, z or 0, w or 0
end)