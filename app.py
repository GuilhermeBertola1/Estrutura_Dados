"""
Este é um cliente ZeroMQ que se conecta a um servidor e envia 4300 solicitações,
aguardando uma resposta após cada uma.
"""
import zmq
import torch

# Verifique se o PyTorch pode acessar a GPU
if torch.cuda.is_available():
    print("CUDA está disponível. Usando GPU:")
    print(f"Nome da GPU: {torch.cuda.get_device_name(0)}")
    print(f"Memória total da GPU: {torch.cuda.get_device_properties(0).total_memory / (1024 ** 3):.2f} GB")
else:
    print("CUDA não está disponível. Usando CPU.")

#zeroMQ
context = zmq.Context()

print("Connecting to hello world server…")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

for request in range(4300):
    print(f"Sending request {request} …")
    socket.send(b"Hello")

    #  Get the reply.
    message = socket.recv()
    print(f"Received reply {request} [ {message} ]")
