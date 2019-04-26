import SimpleHTTPServer
import SocketServer

PORT = 8080
Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
httpd = SocketServer.TCPServer(("", PORT), Handler)
print "Listen on " + str(PORT)
httpd.serve_forever()
