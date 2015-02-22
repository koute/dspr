#!/usr/bin/ruby1.9.1

$done = []
def parse( filename )

    $done << filename

    data = File.read( filename )
    data.gsub!( /#include\s+"(.+?)"/ ) do
        if $done.include? $1
            ''
        else
            parse( $1 )
        end
    end

    data

end

puts parse( "dspr_client.cc" )
puts parse( "sigpipe.cc" )
puts parse( "socket.cc" )
