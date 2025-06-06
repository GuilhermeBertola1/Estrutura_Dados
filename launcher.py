import os
import shutil
import subprocess
import urllib.request
import time
import platform
import sys

URL_DOCKER_WIN = "https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe"
INSTALLER_PATH = "DockerInstaller.exe"

# Git
URL_GIT_WIN = "https://github.com/git-for-windows/git/releases/download/v2.45.1.windows.1/Git-2.45.1-64-bit.exe"
GIT_INSTALLER = "GitInstaller.exe"
GIT_REPO = "https://github.com/GuilhermeBertola1/Estrutura_Dados.git"
REPO_DIR = "Estrutura_Dados"

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

def git_instalado():
    return shutil.which("git") is not None

def baixar_git():
    print("📦 Baixando Git para Windows...")
    urllib.request.urlretrieve(URL_GIT_WIN, GIT_INSTALLER)
    print(f"✅ Instalador salvo como: {GIT_INSTALLER}")

def instalar_git():
    print("🛠️ Iniciando instalação do Git (modo silencioso)...")
    subprocess.run([GIT_INSTALLER, "/VERYSILENT", "/NORESTART"], check=False)
    print("⏳ Aguardando Git finalizar instalação...")
    time.sleep(10)
    os.environ["PATH"] += r";C:\Program Files\Git\cmd"

def clonar_repositorio(destino=REPO_DIR):
    if os.path.isdir(destino):
        print("📁 Repositório já clonado.")
    else:
        print("📥 Clonando repositório do GitHub...")
        subprocess.run(["git", "clone", GIT_REPO], check=True)
        print("✅ Repositório clonado com sucesso.")

def build_rodar_compose():
    print("🔧 Fazendo build de todos os serviços com Docker Compose...")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, REPO_DIR)

    try:
        subprocess.run(
            ["docker", "compose", "-f", "docker-compose.yml", "build"],
            check=True,
            cwd=project_dir
        )

        print("✅ Build concluído.")
        print("🚀 Subindo todos os serviços em segundo plano...")

        subprocess.run(
            ["docker", "compose", "-f", "docker-compose.yml", "up", "-d"],
            check=True,
            cwd=project_dir
        )

        print("🧑‍💻 Entrando no terminal interativo do programa_c...")

        subprocess.run(
            ["docker", "compose", "-f", "docker-compose.yml", "exec", "programa_c", "sh", "-c", "./programa; exec sh"],
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

    if not git_instalado():
        baixar_git()
        instalar_git()
        input("🛠️ Após a instalação do Git, pressione ENTER para continuar...")

    clonar_repositorio()

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