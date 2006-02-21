require 'tree/tree'
require 'body.rb'

#c = TreeNode.new(Vector.new, 2)
#b1 = Body.new(0, 1, Vector.new(0.5, -0.5, 0.5), Vector.new)
#b2 = Body.new(0, 1, Vector.new(-0.5, 0.4, -0.1), Vector.new)
#b3 = Body.new(0, 1, Vector.new(-0.3, 0.9, 0.3), Vector.new)
#b4 = Body.new(0, 1, Vector.new(0.5, -0.4, 0.2), Vector.new)
#c.load_tree(b1)
#c.load_tree(b2)
#c.load_tree(b3)
#c.load_tree(b4)

c = TreeNode.new(Vector.new, 1000)
100.times do
  x = 248*rand(0)
  y = 250*rand(0)
  z = 450*rand(0)
  pos = Vector.new(x,y,z)
  c.load_tree(Body.new(0, 1, pos, Vector.new))
end
c.center_of_mass
c.pp