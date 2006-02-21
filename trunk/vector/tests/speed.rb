require 'Vector'

1000000.times do
  c = Vector.new([rand(10), rand(10), rand(10)])
  c.mag
end