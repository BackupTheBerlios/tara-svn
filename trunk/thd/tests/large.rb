require 'body.rb'
require 'thd_handler.rb'
c = THDHandler.new
time = Time.now
c.load_stream($stdin)
#nbody = c.create_bodies
p Time.now-time
