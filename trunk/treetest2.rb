require 'body.rb'
require 'tree/tree'
require 'vector/vector'

c = TreeNode.new(Vector.new, 2)
b1 = Body.new(0, 1, Vector.new(0.5, -0.5, 0.5), Vector.new)
b2 = Body.new(0, 1, Vector.new(-0.5, 0.4, -0.1), Vector.new)
b3 = Body.new(0, 1, Vector.new(-0.3, 0.9, 0.3), Vector.new)
b4 = Body.new(0, 1, Vector.new(0.5, -0.4, 0.2), Vector.new)
c.load_body(b1)
c.load_body(b2)
c.load_body(b3)
c.load_body(b4)
c.children.compact!

c.each_body {|body| body.pp(2)}
c.each_node {|node| node.pp(2)}
puts ''
c.each_node_and_body{|bn| bn.pp(2)}

c.center_of_mass
p c.mass
c.pos.print