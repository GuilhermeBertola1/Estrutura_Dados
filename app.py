"""
Este é um cliente ZeroMQ que se conecta a um servidor e envia 4300 solicitações,
aguardando uma resposta após cada uma.
"""

import zmq
import streamlit as st

st.title("🛰️ Cliente ZeroMQ")

num_reqs = st.number_input("Número de requisições a enviar", min_value=1, max_value=10000, value=10)

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
