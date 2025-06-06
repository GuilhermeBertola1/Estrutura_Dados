import zmq
import json
import streamlit as st
import pandas as pd
import altair as alt
import datetime
from statsmodels.tsa.api import VAR

st.title("📊 Interface Gráfica - Cliente ZeroMQ")

if "dados" not in st.session_state:
    st.session_state["dados"] = []

# Datas de início e fim
data_inicio = st.date_input("📅 Data de início")
data_fim = st.date_input("📅 Data de fim")

# Opções de hora (inteiras, com AM/PM)
horas_opcoes = []
for h in range(24):
    hora_12 = h % 12
    if hora_12 == 0:
        hora_12 = 12
    sufixo = "AM" if h < 12 else "PM"
    horas_opcoes.append(f"{hora_12:02d}:00:00 {sufixo}")

hora_inicio_str = st.selectbox("🕒 Hora de início (AM/PM)", horas_opcoes, index=0)
hora_fim_str = st.selectbox("🕓 Hora de fim (AM/PM)", horas_opcoes, index=11)

# Combina data + hora (em string)
data_hora_inicio_str = f"{data_inicio.strftime('%Y-%m-%d')} {hora_inicio_str}"
data_hora_fim_str = f"{data_fim.strftime('%Y-%m-%d')} {hora_fim_str}"

# Converte para datetime para validar
try:
    data_hora_inicio = datetime.datetime.strptime(data_hora_inicio_str, "%Y-%m-%d %I:%M:%S %p")
    data_hora_fim = datetime.datetime.strptime(data_hora_fim_str, "%Y-%m-%d %I:%M:%S %p")
except ValueError:
    st.error("❌ Erro ao converter data e hora.")
    data_hora_inicio = data_hora_fim = None

if data_hora_inicio and data_hora_fim and data_hora_inicio > data_hora_fim:
    st.warning("⚠️ A data e hora de início devem ser anteriores à data e hora de fim.")

if st.button("🔄 Requisitar dados por intervalo"):
    st.write("🔌 Conectando ao servidor ZeroMQ...")
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

    # Envia as strings formatadas no formato exato
    msg = f"{data_hora_inicio.strftime('%Y-%m-%d %I:00:00 %p')},{data_hora_fim.strftime('%Y-%m-%d %I:00:00 %p')}"
    print(msg)
    st.write(f"📨 Enviando: {msg}")
    socket.send_string(msg)

    resposta = socket.recv().decode()
    try:
        dados = json.loads(resposta)
        if isinstance(dados, list):
            st.session_state["dados"] = dados
            print(st.session_state["dados"])
            st.success("✅ Dados recebidos com sucesso.")
        else:
            st.error("❌ Formato de resposta inválido.")
    except json.JSONDecodeError:
        st.error(f"❌ Erro ao decodificar JSON: {resposta}")

    socket.close()
    context.term()

# Exibição dos dados e gráfico
if st.session_state["dados"]:
    df = pd.DataFrame(st.session_state["dados"])
    print(df["data"])
    df["data"] = pd.to_datetime(df["data"], format="%Y-%m-%d %I:%M:%S %p")
    df = df.sort_values("data")
    print(df["data"])

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

    escolha = st.selectbox("📈 Escolha a variável para o gráfico:", colunas, key="grafico_real")

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

    # Caixa para predição
    # Caixa para predição
    st.subheader("🔮 Predição com VAR")
    data_inicio_pred = st.date_input("📅 Início da predição", key="pred_ini")
    data_fim_pred = st.date_input("📅 Fim da predição", key="pred_fim")

    if st.button("🚀 Treinar e Prever"):
        try:
            df = df.set_index("data")
            df = df.apply(pd.to_numeric, errors="coerce")
            df = df.dropna(axis=1, how="all").dropna()

            modelo = VAR(df)
            lag_order = modelo.select_order(13).hqic
            resultados = modelo.fit(lag_order)

            n_passos = (data_fim_pred - data_inicio_pred).days + 1
            previsao = resultados.forecast(df.values[-lag_order:], steps=n_passos)

            df_prev = pd.DataFrame(previsao, columns=df.columns)
            datas_prev = pd.date_range(start=data_inicio_pred, periods=n_passos, freq="D")
            df_prev["data"] = datas_prev

            st.session_state["df_prev"] = df_prev

            st.success("✅ Predição gerada com sucesso.")
        except Exception as e:
            st.error(f"Erro ao treinar o modelo VAR: {e}")

    # Se houver predição salva, mostrar gráfico
    if "df_prev" in st.session_state:
        df_prev = st.session_state["df_prev"]
        st.write("📈 Previsão:")
        st.dataframe(df_prev)

        escolha1 = st.selectbox("📈 Escolha a variável para o gráfico:", colunas, key="grafico_prev")

        grafico_prev = alt.Chart(df_prev).mark_line().encode(
            x="data:T",
            y=escolha1,
            tooltip=["data", escolha1]
        ).properties(
            width=800,
            height=400,
            title=f"Previsão de {escolha1}"
        )

        st.altair_chart(grafico_prev, use_container_width=True)
