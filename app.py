"""
Este é um cliente ZeroMQ que se conecta a um servidor e envia 10 solicitações,
aguardando uma resposta após cada uma.
"""
import zmq

context = zmq.Context()

#  Socket to talk to server
print("Connecting to hello world server…")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

#  Do 10 requests, waiting each time for a response
for request in range(4300):
    print(f"Sending request {request} …")
    socket.send(b"Hello")

    #  Get the reply.
    message = socket.recv()
    print(f"Received reply {request} [ {message} ]")
