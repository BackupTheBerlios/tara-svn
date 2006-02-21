require 'body.rb'
require 'thd_handler.rb'
c = THDHandler.new
c.load_stream($stdin)
nbody = c.create_bodies
p nbody.list
puts nbody.stream
