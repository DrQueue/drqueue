$, = " "; $\ = "\n"
require 'python';
require 'python/BaseHTTPServer'
include Py::BaseHTTPServer

def test(handlerClass = BaseHTTPRequestHandler,
         serverClass = HTTPServer)
  # Test the HTTP request handler class.
  #
  # This runs an HTTP server on port 8000 (or the first command line
  # argument).

  if ARGV[0]
    port = ARGV[0].to_i
  else
    port = 8000
  end
  server_address = Py.Tuple('', port)

  httpd = serverClass.new(server_address, handlerClass)

  print "Serving HTTP on port", port, "..."
  httpd.serve_forever()
end


if __FILE__ == $0
  test()
end
