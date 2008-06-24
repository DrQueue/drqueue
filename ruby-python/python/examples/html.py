import httplib
import htmllib
import formatter

import re, sys

match = None
if len(sys.argv) > 1:
    url = sys.argv[1]
    match = re.match('^http:\/\/([^\/]+)(\/.*)$', url)
if match:
    host, path = match.groups()
else:
    print "Usage: python ", sys.argv[0], " http://host/[path]"
    sys.exit(1)

h = httplib.HTTP(host)
h.putrequest('GET', path)
h.putheader('Accept', 'text/html')
h.putheader('Accept', 'text/plain')
h.endheaders()
errcode, errmsg, headers = h.getreply()

if errcode == 200:
    data = h.getfile().read()
    
    f = formatter.AbstractFormatter(formatter.DumbWriter())
    parser = htmllib.HTMLParser(f)
    parser.feed(data)
    parser.close()
else:
    print errcode, ": Failed to fetch", url
    sys.exit(1)
