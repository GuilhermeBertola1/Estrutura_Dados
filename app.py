import zmq
import torch
import streamlit as st

# Título
st.title("🛰️ Cliente ZeroMQ com PyTorch")

# GPU ou CPU
if torch.cuda.is_available():
    st.success("CUDA está disponível ✅")
    st.write(f"Nome da GPU: {torch.cuda.get_device_name(0)}")
    st.write(f"Memória total: {torch.cuda.get_device_properties(0).total_memory / (1024 ** 3):.2f} GB")
else:
    st.warning("CUDA não disponível. Usando CPU.")

# Número de requisições
num_reqs = st.number_input("Número de requisições a enviar", min_value=1, max_value=10000, value=10)

# Botão para enviar requisições
if st.button("Enviar requisições"):
    st.write("🔌 Conectando ao servidor ZeroMQ...")

    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")
    progresso = st.progress(0)
    mensagens = st.empty()

    for i in range(num_reqs):
        socket.send(b"Hello")
        reply = socket.recv()
        mensagens.text(f"📨 [{i+1}] Resposta recebida: {reply.decode()}")
        progresso.progress((i + 1) / num_reqs)

    st.success("✅ Todas as requisições foram enviadas e respostas recebidas.")
