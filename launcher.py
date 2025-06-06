import os
import shutil
import subprocess
import urllib.request
import time
import platform
import sys

URL_DOCKER_WIN = "https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe"
INSTALLER_PATH = "DockerInstaller.exe"

# URL do docker-compose.yml no seu GitHub
URL_COMPOSE = "https://raw.githubusercontent.com/GuilhermeBertola1/Estrutura_Dados/main/docker-compose.yml"
URL_DOCKERFILE_C = "https://raw.githubusercontent.com/GuilhermeBertola1/Estrutura_Dados/main/Dockerfile.c"
URL_DOCKERFILE_PY = "https://raw.githubusercontent.com/GuilhermeBertola1/Estrutura_Dados/main/Dockerfile.streamlit"

def docker_instalado():
    return shutil.which("docker") is not None

def baixar_instalador():
    print("📦 Baixando Docker Desktop Installer...")
    urllib.request.urlretrieve(URL_DOCKER_WIN, INSTALLER_PATH)
    print(f"✅ Instalador salvo como: {INSTALLER_PATH}")

def executar_instalador():
    print("🚀 Abrindo instalador do Docker. Siga as instruções na tela...")
    subprocess.Popen([INSTALLER_PATH], shell=True)

def abrir_docker_desktop():
    docker_desktop_path = r"C:\Program Files\Docker\Docker\Docker Desktop.exe"
    if os.path.isfile(docker_desktop_path):
        try:
            subprocess.Popen([docker_desktop_path])
            print("🐳 Docker Desktop iniciado automaticamente.")
        except Exception as e:
            print("⚠️ Falha ao iniciar Docker Desktop automaticamente:", e)
    else:
        print(f"⚠️ Docker Desktop não encontrado em: {docker_desktop_path}")
        print("Por favor, abra o Docker Desktop manualmente.")

def docker_funcionando():
    try:
        subprocess.run(["docker", "info"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def aguardar_docker_ativo():
    print("⏳ Aguardando Docker Desktop iniciar e ficar ativo...")
    tentativas = 0
    while not docker_funcionando():
        tentativas += 1
        print(f"🔄 Docker não está pronto (tentativa {tentativas})... tentando novamente em 5 segundos.")
        time.sleep(5)
        if tentativas >= 30:
            print("⚠️ Tempo limite atingido. Certifique-se que o Docker Desktop está aberto e funcionando.")
            break
    if docker_funcionando():
        print("✅ Docker está ativo!")

def obter_diretorios():
    if getattr(sys, 'frozen', False):
        script_dir = os.path.dirname(sys.executable)
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))

    project_dir = os.path.abspath(os.path.join(script_dir, ".."))
    return script_dir, project_dir

def verificar_ou_baixar_compose(compose_path):
    if not os.path.isfile(compose_path):
        print("📄 docker-compose.yml não encontrado. Baixando do GitHub...")
        try:
            urllib.request.urlretrieve(URL_COMPOSE, compose_path)
            print("✅ docker-compose.yml baixado com sucesso.")
        except Exception as e:
            print(f"❌ Erro ao baixar docker-compose.yml: {e}")
            return False
    return True

def build_rodar_compose():
    print("🔧 Fazendo build de todos os serviços com Docker Compose...")

    script_dir, project_dir = obter_diretorios()
    compose_path = os.path.join(project_dir, "docker-compose.yml")

    if not verificar_ou_baixar_compose(compose_path):
        return

    try:
        subprocess.run(
            ["docker", "compose", "-f", compose_path, "build"],
            check=True,
            cwd=project_dir
        )

        print("✅ Build concluído.")
        print("🚀 Subindo todos os serviços em segundo plano...")

        subprocess.run(
            ["docker", "compose", "-f", compose_path, "up", "-d"],
            check=True,
            cwd=project_dir
        )

        print("🧑‍💻 Entrando no terminal interativo do programa_c...")

        subprocess.run(
            ["docker", "compose", "-f", compose_path, "exec", "programa_c", "sh", "-c", "./programa; exec sh"],
            check=True,
            cwd=project_dir
        )

        print("✅ Execução finalizada.")

    except subprocess.CalledProcessError as e:
        print("❌ Erro ao executar docker compose:", e)

def main():
    if platform.system() != "Windows":
        print("⚠️ Este script automático funciona apenas no Windows.")
        return

    if not docker_instalado():
        baixar_instalador()
        executar_instalador()
        input("🛠️ Após instalar o Docker Desktop, pressione ENTER para tentar iniciar o Docker...")
        abrir_docker_desktop()
        aguardar_docker_ativo()
    else:
        print("✅ Docker já está instalado.")
        if not docker_funcionando():
            print("⚠️ Docker instalado, mas não está rodando. Tentando iniciar Docker Desktop...")
            abrir_docker_desktop()
            aguardar_docker_ativo()

    if docker_funcionando():
        try:
            build_rodar_compose()
        except Exception as e:
            print("❌ Erro ao executar comandos Docker Compose:", e)
    else:
        print("❌ Docker não está ativo. Não é possível rodar a aplicação.")

    input("Pressione Enter para sair...")

if __name__ == "__main__":
    main()