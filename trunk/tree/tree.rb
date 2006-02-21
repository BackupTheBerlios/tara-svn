class TreeNode
  attr_accessor :size, :center, :children, :mass, :pos
  def initialize(center, size)
    @center, @size = center, size
    @children = Array.new(8)
    @mass = 0; @pos = Vector.new
  end

  # returns an octant for the load_body routine
  # written by J.Makino
  def octant(pos)
    octant = 0
    pos.each_index do |i| 
      octant *= 2
      octant += 1 if pos[i] > @center[i]
    end
    octant
  end
  
  # returns a center offset for the creation of new nodes
  # written by J.Makino
  def offset(corner)
    r = []
    3.times{ r.unshift( (corner & 1)*2 - 1 ) ; corner>>=1 }
    r.to_v
  end

  # loads a body into the tree
  def load_body(b)
    corner = octant(b.pos)
    if @children[corner] == nil
      @children[corner] = b
      return
    elsif @children[corner].class == Body
      prev_b = @children[corner]
      new_node_size = @size/2.0
      new_center_offset = @center + offset(corner)*new_node_size
      @children[corner] = TreeNode.new(new_center_offset, new_node_size)
      @children[corner].load_body(prev_b)
      @children[corner].load_body(b)
    end
  end
  
  # for each body in the tree, execute a code block
  def each_body(&block)
    @children.each do |child|
      if child.class == Body
        yield(child)
      elsif child.class == TreeNode
        child.each_body(&block)
      end
    end
  end
  
  # for each node in the tree, execute a code block
  def each_node(&block)
    tot = 0
    tot = yield(self) if self.class == TreeNode
    @children.each do |child|
      if child.class == TreeNode
        child.each_node(&block)
      end
    end
  end
  
  # for each node and body in the tree, execute a code block
  def each_node_and_body(&block)
    tot = 0
    tot = yield(self) if self.class == TreeNode
    @children.each do |child|
      if child.class == Body then
        tot += yield(child)
      elsif child.class == TreeNode then
        tot += child.each_node_and_body(&block)
      end
    end
    tot
  end
  
  def pp(indent=2)
    puts " "*indent+
         "<NODE: children= #{@children.compact.size} | size= #{@size} | "+
         "center = [#{@center[0]}, #{@center[1]}, #{@center[2]}]>"
  end
end