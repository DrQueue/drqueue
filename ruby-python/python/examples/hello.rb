require 'python'
require 'python/sys'

Py::Sys.stdout.write("Hello python world!\n")

Py.eval 'sys.stdout.write("Hello ruby world!\n")'
