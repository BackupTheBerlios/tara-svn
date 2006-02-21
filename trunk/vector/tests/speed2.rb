require 'Vector'

100000.times do
  c = Vector.new([rand(10),rand(10),rand(10)])
  d = Vector.new([rand(10),rand(10),rand(10)])
  ((c+d)).cross(d).mag
end