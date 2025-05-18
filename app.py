import zmq
import json
import streamlit as st
import pandas as pd
import altair as alt

st.title("📊 Interface Gráfica - Cliente ZeroMQ")

if "dados" not in st.session_state:
    st.session_state["dados"] = []

data_inicio = st.date_input("📅 Data de início")
data_fim = st.date_input("📅 Data de fim")

if data_inicio > data_fim:
    st.warning("⚠️ A data de início deve ser anterior à data de fim.")

if st.button("🔄 Requisitar dados por intervalo de datas"):
    st.write("🔌 Conectando ao servidor ZeroMQ...")
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

    msg = f"{data_inicio.strftime('%Y-%m-%d')},{data_fim.strftime('%Y-%m-%d')}"
    socket.send_string(msg)
    
    resposta = socket.recv().decode()
    try:
        dados = json.loads(resposta)
        if isinstance(dados, list):
            st.session_state["dados"].extend(dados)
            st.success("✅ Dados recebidos com sucesso.")
        else:
            st.error("❌ Formato de resposta inválido.")
    except json.JSONDecodeError:
        st.error(f"❌ Erro ao decodificar JSON: {resposta}")

    socket.close()
    context.term()

if st.session_state["dados"]:
    df = pd.DataFrame(st.session_state["dados"])
    df["data"] = pd.to_datetime(df["data"])

    st.write("🧾 Dados Recebidos:")
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

    escolha = st.selectbox("📈 Escolha a variável para o gráfico:", colunas)

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
