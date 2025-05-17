import zmq
import json
import streamlit as st
import pandas as pd
import altair as alt

st.title("📊 Interface Grafica")

if "dados" not in st.session_state:
    st.session_state["dados"] = []

num_reqs = st.number_input("Número de requisições a enviar", min_value=1, max_value=10000, value=10)

# Botão para buscar dados do servidor
if st.button("Requesicao dos dados do servidor"):
    st.write("🔌 Conectando ao servidor ZeroMQ...")
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

    progresso = st.progress(0)
    dados = []

    for i in range(num_reqs):
        socket.send(b"Hello")
        resposta = socket.recv().decode(errors="ignore").strip().rstrip('\x00')

        try:
            json_data = json.loads(resposta)
            dados.append(json_data)
        except json.JSONDecodeError:
            st.error(f"Erro ao decodificar JSON: {resposta}")

        progresso.progress((i + 1) / num_reqs)

    socket.close()
    context.term()

    # Salva os dados na sessão para não perder ao interagir
    st.session_state["dados"].extend(dados)
    st.success("✅ Dados recebidos e salvos com sucesso.")

# Só continua se já tiver dados salvos
if "dados" in st.session_state and st.session_state["dados"]:
    df = pd.DataFrame(st.session_state["dados"])
    df["data"] = pd.to_datetime(df["data"])  # garante tipo datetime

    st.write("📅 Visualização dos dados recebidos:")
    st.dataframe(df)

    colunas = [
        "demanda_residual",
        "demanda_contratada",
        "geracao_despachavel",
        "geracao_termica",
        "importacoes",
        "geracao_renovavel_total",
        "carga_reduzida_manual",
        "capacidade_instalada",
        "perdas_geracao_total"
    ]

    escolha = st.selectbox("Escolha a variável para o gráfico:", colunas)

    grafico = alt.Chart(df).mark_line(point=True).encode(
        x="data:T",
        y=escolha,
        tooltip=["data", escolha]
    ).properties(
        width=800,
        height=400,
        title=f"{escolha} ao longo do tempo"
    )

    st.altair_chart(grafico, use_container_width=True)
