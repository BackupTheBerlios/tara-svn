puts '<space>'
puts "\t<cluster>"

10000.times do
  puts "\t\t<body mass=\"#{rand(100)}\" pos=\"#{rand(100)} #{rand(321)} #{rand(31)}\" vel=\"#{rand(10)} #{rand(10)} #{rand(10)}\"/>"
end
puts "\t</cluster>"
puts '</space>'
