#!/usr/bin/env ruby

# sets up the files
dirs = ["pairwise/", "tree/", "vector/"]
dirs.each do |dir|
  Dir.chdir(dir)
  puts "creating extensions in #{dir}"
  `ruby extconf.rb`
  puts `make`
  Dir.chdir("..")
end