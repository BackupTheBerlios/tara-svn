require 'tree/tree'
require 'vector/vector'
require 'pairwise/pairwise'
require 'body.rb'
require 'thd/thd_handler.rb'
require 'parser.rb'


# PARSER ----------------------------------------------------
help_header = <<ENDSTR
tara < fig8.thd
Integrates the dynamical equations of motion for a small-N system specified 
by a .thd file.

ENDSTR

parser = Parser.new
parser.load ['-h', '--help', 'prints out help',
  Proc.new{ parser.print_help(help_header) }, false, 0]

@dt = 0.01
parser.load ['-dt', '--timestep', 'the integration timestep: <float>',
  Proc.new{ |arg| @dt = arg.to_f }, true, 1]

@t_start = 0
parser.load ['-ts', '--time_start', 
  'the system time when starting integration: <float>',
  Proc.new{ |arg| @t_start = arg.to_f }, true, 1]

@t_end = 10
parser.load ['-te', '--time_end', 
  'the system time when stopping integration: <float>',
  Proc.new{ |arg| @t_end = arg.to_f }, true, 1]

@out_dt = 0.1
parser.load ['-o', '--output_interval', 'write-out interval: <float>',
  Proc.new{ |arg| @out_dt = arg.to_f }, false, 1]
  
@eps = 0.0
parser.load ['-e', '--epsilon', 'softening parameter: <float>',
  Proc.new{ |arg| @eps = arg.to_f }, false, 1]
  
@tol = 0.5
parser.load ['-tol', '--opening_tolerance', 
  'the opening tolerance that determines whether a node is to be opened '+
  'up for the force calculation: <float>',
  Proc.new{ |arg| @tol = arg.to_f }, false, 1]

@integrator = 'leapfrog'
parser.load ['-i', '--integrator', 'the integrator to use <leapfrog>',
  Proc.new{ |arg| @integrator = arg }, false, 1]
  
@step_out = false
parser.load ['-s', '--output_files', 'turns on file output',
  Proc.new{ @step_out = true; warn 'outputting data' }, false, 0]
  
@use_tree = true
parser.load ['-nt', '--no_tree', 'disables the B&H tree',
  Proc.new{ @use_tree = false; warn 'tree disabled' }, false, 0]

parser.parse_argv()
# ______________________________________ END PARSER

thd = THDHandler.new
thd.load_stream($stdin)
nbody = thd.create_bodies
nbody.set_parameters(@dt, @t_start, @t_end, @out_dt, @eps, 
                     @step_out, @use_tree)
warn "START energy: #{nbody.energy}" 
nbody.evolve(@integrator, @tol)
warn "END energy: #{nbody.energy}"