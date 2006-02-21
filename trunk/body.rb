=begin rdoc
This class is the lowest level object in Tara. Every astronomical object
is a Body, distinguished only by its 'type' and the larger object that
it 'belongs_to'. Things floating in free space belong to 'space'
=end
class Body
  require 'vector/vector'
  require 'pairwise/pairwise'
  attr_accessor :id, :mass, :pos, :vel, :acc
  
  def initialize(id, mass=1.0, pos=Vector.new,
                 vel=Vector.new, belongs_to='space', 
                 type='star')
    @id, @mass = id, mass
    @belongs_to = belongs_to
    @type = type
    @pos, @vel = Vector.new(*pos), Vector.new(*vel)
    @acc = Vector.new
  end
  
  def kick(dt)
    @vel += @acc*(0.5*dt)
  end
  
  def drift(dt)
    @pos += @vel*dt
  end
  
  def e_kin
    @vel*@vel*@mass*0.5
  end
  
  def e_pot(body_list, eps)
    epot = 0
    body_list.each do |other_body|
      unless other_body == self
        epot += SpeedUp::get_potential_energy(@mass, other_body.mass,
          @pos, other_body.pos, eps)
      end
    end
    epot
  end
  
  def pp(indent = 2)
    print " "*indent + to_s + "\n"
  end
  
  def to_s
    "mass = " + @mass.to_s + 
    "   pos = " + "[#{@pos.to_a.join(", ")}]" + 
    "   vel = " + "[#{@vel.to_a.join(", ")}]"
  end
  
  def get_acc(other, tol, eps)
    acc = Vector.new
    if self != other then
      acc = SpeedUp::pairwise_acc(@pos, other.pos, other.mass, eps) end
    acc
  end
  
end

=begin rdoc
This class consists of the list of particles that will be integrated by
Tara. Also, it consists of the THD stream, as it relates to each instant
of time. The idea is that at the end of the integration, you should just
be able to print out the stream
=end
class NBody
  require 'vector/vector'
  require 'rexml/document'
  require 'pairwise/pairwise'
  include REXML
  
  attr_accessor :stream, :list
  def initialize(stream=nil, list=[])
    @stream = stream  # this is the open REXML document stream
    @list = list      # the array of particles
  end
  
  def set_parameters(dt, t_start, t_end, out_dt, eps, step_out, use_tree)
    @dt = dt; @t_start = t_start; @t_end = t_end; @out_dt = out_dt;
    @eps = eps; @step_out = step_out; @use_tree = use_tree
  end
  
  # evolves the system over time
  def evolve(integrator, tol)
    @tol = tol if tol
    time = @t_start
    print_time = @out_dt
    
    init_acc
    while time <= @t_end
      #warn '------------------'
      send(integrator)
      write_data()
      time += @dt
    end
  end
  
  # write out data
  def write_data
    @list.each do |b|
      puts "#{b.pos[0]} #{b.pos[1]} #{b.pos[2]}"
    end
  end
  
  # initializes the accelerations
  def init_acc
    @list.each {|b| b.acc = pairwise_acc(b) }
  end
  
  # computes the pairwise acceleration for a single body
  # uses the SpeedUp module's fast inner loop function (pairwise.c)
  def pairwise_acc(b)
    acc = Vector.new
    @list.each do |other_body|
      unless other_body == b
        acc += SpeedUp::pairwise_acc(b.pos, other_body.pos,
                                     other_body.mass, @eps)
      end
    end
    acc
  end
  
  def energy
    tot_kin = tot_pot = 0
    @list.inject {|tot_kin, body| body.e_kin}
    @list.inject {|tot_pot, body| body.e_pot(@list, @eps)}
    tot_kin + tot_pot
  end
    
  # the leapfrog integrator. kick, drift, acc, kick!
  def leapfrog
    @list.each {|b| b.kick(@dt) }
    @list.each {|b| b.drift(@dt) }
    #warn 'kick+drift done'
      if @use_tree == false then
        @list.each {|b| b.acc = pairwise_acc(b) } 
      else 
        get_tree_acc(@tol)
      end
    @list.each {|b| b.kick(@dt) }
    #warn 'final kick done'
  end
  
  def make_tree
    r = @list.inject(0){|max, b| [max, b.pos.abs.max].max}
    root_size = 1
    root_size *= 2 while r > root_size
    @root_node = TreeNode.new(Vector.new, root_size);
    @list.each { |b| @root_node.load_tree(b) }
  end
  
  def get_tree_acc(tol)
    make_tree
    #warn 'tree generated'
    @root_node.center_of_mass
    #warn 'center of mass computed'
    @list.each do |b| 
      b.acc = @root_node.get_acc(b, tol, @eps)
    end
    #warn 'accelerations computed'
  end

  # adds an entry to the THD stream history
  def add_to_history(new_entry)
    if !@stream.nil? then
      # if there's no <history>, then add a new tag to 
      # the THD stream
      if @stream.root.elements['history'].to_a == [] then
        @stream.root.add_element('history') 
      end
      # modify the <history> tag
      XPath.each( @stream, '//history') do |hist|
        # create an <entry>, add text, and add it to <history>
        hist_entry = Element.new('entry')
        hist_entry.add_text(new_entry)
        hist << hist_entry
      end
    else
      raise "\nNo stream loaded!\n" 
    end
  end
end

#class TreeNode
#  def pp(indent=0)
#    puts " "*indent+"node size=#{self.size}"
#    self.each_child {|x| x.pp(2+indent) if x }
#  end
#  
#  def get_acc(b, tol, eps)
#    dist = b.pos - @pos
#    if 2*@size > tol*dist.mag then
#      acc = Vector.new
#      self.each_child do |c|
#        acc += c.get_acc(b, tol, eps) if c
#      end
#      acc
#    else
#      SpeedUp::pairwise_acc(c.pos, b.pos, b.mass, eps) if c.pos != b.pos
#    end
#  end
#end

# modified Array to have a vector conversion class
class Array
  def to_v
    Vector.new(self[0], self[1], self[2])
  end
end