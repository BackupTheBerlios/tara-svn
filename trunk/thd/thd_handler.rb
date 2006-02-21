=begin 
Tara ->> thd_handler.rb
author: Pradeep Elankumaran, (c) 2006

CHANGELOG: 
  15, Jan 2006 - first started
  
SAMPLE:
  require 'body.rb'
  require 'thd_handler.rb'
  c = THDHandler.new
  c.load_stream($stdin)
  nbody = c.create_bodies
  p nbody.list
  puts nbody.stream
    >>>> then run as: ruby scriptname < fig8.thd
NOTES:
This handler assumes that the .thd files follow the XML schema
given in TARA_DIR/thd/SCHEMA
=end

=begin rdoc
This class has all the methods to handle the input and output
files that have the .thd (Tara Hierarchical Dataset) extensions
=end

class THDHandler

  # the REXML require's
  require 'rexml/document'
  include REXML
  
  # loads an XML stream specified by a given filename, or $stdin
  def load_stream(filename)
    if filename != $stdin then
      stream = File.new(filename)
    else stream = filename end 
    @doc = Document.new(stream)
    $stderr.puts('stream loaded')
  end
  
  # returns the stream associated with the handler
  def to_s
    @doc.to_s
  end
  
  # after the stream has been loaded, pulls out all the 'body'
  # nodes, makes Body's and returns an NBody Object.
  def create_bodies()
    if @doc.nil? then raise "\nNo stream loaded!" end
    list = []
    # '//body' pulls out all the 'body' nodes regardless
    # of where they are in the document
    XPath.each( @doc, '//body' ) do |body|
      id = body.attributes['id'].to_i
      mass = body.attributes['mass'].to_f
      pos = body.attributes['pos'].split.collect!{|x| x.to_f}
      vel = body.attributes['vel'].split.collect!{|x| x.to_f}
      type = body.attributes['type']
      
      # if the bodies are located one node below <space>
      if body.parent.parent.name == 'space' then
        belongs_to = body.parent.name
      # or if the bodies are N-nodes below <space>,
      # keep loading the names of their parent nodes into their
      # family tree, till the first node below <space> is found
      else
        belongs_to = []
        c = body.parent
        while c.name != 'space' do
          belongs_to.push(c.name)
          c = c.parent
        end
        # reverse ruby array behavior to reflect the 
        # proper heirarchy, with the first node below <space>
        # at the beginning
        belongs_to.reverse!
      end
      new_body = Body.new(id, mass, pos, vel, belongs_to, type)
      list.push(new_body)
    end
    # loads the open document and the list into a new NBody Object
    new_nbody_obj = NBody.new(@doc, list)
    # modifies the history
    new_nbody_obj.add_to_history('accessed by THDHandler')
    new_nbody_obj
  end

end