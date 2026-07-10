# :robot: Aula de Introdução ao ROS 2 - Perse Sumo for dummies :robot:

Este repositório contém o código de acompanhamento do para a aulinha de ROS 2, onde o objetivo é brincar com o perse numa partida de sumô!

## :wrench: Instalação

### :whale: Usando Docker

Caso você ~~não seja psicopata~~ deseje obter o ambiente já configurado, sem necessidade de instalação, você pode usar o docker, que garante que todas as dependências estarão corretas.

1. **Construa a imagem do docker:**
   ```bash
   docker compose build
   ```

2. **Inicie o container:**
   ```bash
   docker compose up -d
   ```

3. **Acesse o terminal do container:** O serviço é nomeado `perse` no arquivo `docker compose.yml`.
   ```bash
   docker compose exec perse bash
   ```

4. **Dentro do container, compile o projeto:** O código já estará no diretório `/home/ros/perse_ws`.
   ```bash
   cd /home/ros/perse_ws
   colcon build
   ```

### :computer: Utilizando ROS nativamente:

1. Instale a versão específica de ROS 2 para seu sistema operacional seguindo as instruções do site oficial (agora com suporte à Windows, yay!).
   
   >:link: Link Site Oficial: [ROS 2 - Getting Started](https://www.ros.org/blog/getting-started/)

2.  Caso você tenha múltiplas versões de ROS instaladas, é necessário ativar a versão desejada em todo novo terminal pelo comando:

    ```bash
    source /opt/ros/NOME_DA_VERSAO/setup.bash
    ```

    Por exemplo, se sua versão for a lyrical (Ubuntu 26), o comando fica:

    ```bash
    source /opt/ros/lyrical/setup.bash
    ```

    Caso você só tenha uma única versão do ROS instalada ou desejada (nosso caso), você pode adicionar o comando no arquivo de inicialização do bash.

    ```bash
    echo 'source /opt/ros/NOME_DA_VERSAO/setup.bash' >> ~/.bashrc
    ```

3. Criando uma workspace:
   
   Para trabalhar com ROS, precisamos de uma workspace e sua pasta source, onde ficarão os pacotes:

   ```bash
   mkdir -p ~/perse_ws/src
   # Criamos a pasta perse_ws (workspace) e src (source)
   cd ~/perse_ws/src
   # Entra na pasta
   ```
   Dentro da source, clonamos os pacotes desejados. 

   ```bash
   git clone https://github.com/ThundeRatz/aula-ros-2026.git
   ```

4. Na pasta da workspace, instale as dependências do ROS necessárias utilizadno o rosdep:

   ```bash
   cd ..
   rosdep install -i --from-path src --rosdistro NOME_DA_VERSAO -y
   ```

5. Compile o workspace:
   
   ```bash
   colcon build
   ```

---
## :rocket: Executando o projeto

Após a compilação, abra dois terminais. Certifique-se de estar no ambiente correto:

- Se estiver utilizando Docker, acesse o container com ```docker compose exec perse bash```
- Se estiver usando instalação nativa, execute ```source /install/setup.bash``` dentro da pasta da workspace **em cada um dos terminais.**

1. **Terminal 1: Inicie a simulação.** Este comando inicia o webots, ambiente de simulação 3D, com o perse no dohyo.
   
   ```bash
   ros2 launch perse_sumo sim_launch.py
   ```

2. **Terminal 2: Inicie o Cérebro do Robô.** Este comando executa o nó de controle que você irá modificar.
   
   ```bash
   ros2 launch perse_brain brain.launch
   ```

Agora, você está pronto para modificar o código em ```src/perse_brain/src/brain.cpp``` e ver o resultado na simulação!




## :open_file_folder: Estrutura de arquivos

O workspace está organizado em dois pacotes principais: um do cérebro e um da simulação:

```bash
.
├── perse_brain/         # Pacote com a lógica de controle do robô
│   ├── CMakeLists.txt
│   ├── package.xml
│   ├── config/
│   │   └── brain_parameters.yaml  # Parâmetros de configuração do cérebro
│   ├── include/
│   │   └── perse_brain/
│   │       └── brain.hpp          # Arquivo de cabeçalho do nó
│   ├── launch/
│   │   └── brain.launch           # Arquivo de lançamento para o cérebro
│   └── src/
│       └── brain.cpp              # Onde a mágica acontece! Código principal do robô
└── perse_sumo/          # Pacote da simulação
    ├── CMakeLists.txt
    ├── package.xml
    ├── launch/
    │   └── sim_launch.py          # Arquivo de lançamento para a simulação
    ├── protos/                    # Modelos 3D para o Webots
    ├── resource/
    │   └── perse.urdf             # Descrição do robô (sensores, links)
    ├── src/
    │   └── PerseDriver.cpp        # Driver que conecta o Webots com o ROS 2
    └── worlds/
        └── world.wbt              # Arquivo do mundo da simulação
```