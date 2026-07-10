## Arguments Definition
# ARG ROS_IMAGE=ros:humble-ros-base
# ARG ROS_IMAGE=dustynv/ros:humble-ros-base-l4t-r32.7.1
ARG ROS_IMAGE=osrf/ros:humble-desktop-full

# Crete non-root user arguments
ARG USERNAME=thor
ARG USER_UID=1000
ARG USER_GID=1000
ARG RENDER_GROUP_ID=1001

## Stage 1: Setup and Base Dependencies (Setup Stage)

FROM $ROS_IMAGE AS base_builder

ARG USERNAME
ARG USER_UID
ARG USER_GID
ARG RENDER_GROUP_ID

# Set default environment variables
ENV PERSE_WS /home/${USERNAME}/perse_ws
ENV USER_HOME /home/${USERNAME}
ENV DEBIAN_FRONTEND noninteractive

# Create user and groups
RUN groupadd --gid ${USER_GID} ${USERNAME} && \
    useradd --uid ${USER_UID} --gid ${USER_GID} -m ${USERNAME} && \
    usermod --shell /bin/bash ${USERNAME} && \
    usermod -aG video ${USERNAME} && \
    apt-get update && \
    apt-get install -y sudo && \
    sudo rm -rf /var/lib/apt/lists/* && \
    echo ${USERNAME} ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME}

USER ${USERNAME}

RUN mkdir -p ${PERSE_WS}/src
WORKDIR ${PERSE_WS}

# Install general non-ROS dependencies
RUN sudo apt-get update --fix-missing && \
    sudo apt-get install -y libboost-all-dev libx11-dev wget ripgrep && \
    sudo rm -rf /var/lib/apt/lists/*

# Install fish and plugins
RUN sudo apt-get update --fix-missing && \
    sudo apt-get install -y less software-properties-common && \
    sudo apt-add-repository ppa:fish-shell/release-3 && \
    sudo apt-get install -y fish curl && \
    fish -c "curl -sL https://raw.githubusercontent.com/jorgebucaran/fisher/main/functions/fisher.fish | source && fisher install\
        jorgebucaran/fisher\
        edc/bass\
        pure-fish/pure\
        jorgebucaran/autopair.fish\
        jorgebucaran/nvm.fish\
        PatrickF1/fzf.fish" && \
    fish -c "git clone --depth 1 https://github.com/junegunn/fzf.git ~/.fzf; ~/.fzf/install" && \
    sudo rm -rf /var/lib/apt/lists/*

# Install node lts
RUN fish -c 'nvm install lts; set --universal nvm_default_version lts'

# set up environment
COPY update_bashrc /sbin/update_bashrc
RUN sudo chmod +x /sbin/update_bashrc; sudo chown ${USERNAME} /sbin/update_bashrc; sync; /bin/bash -c /sbin/update_bashrc; sudo rm /sbin/update_bashrc

## Stage 2: ROS Package Metadata and Dependencies (Cacher & Dependencies Stages)

# Note: The original Dockerfile used separate Cacher stages for better build caching.
# This merged version copies all package.xml files and uses conditional logic later.

# Copy all common package.xml files
COPY perse_brain/package.xml src/perse_brain/
COPY perse_sumo/package.xml src/perse_sumo/

ENV AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS 1

# Install ROS Dependencies (Simulating dependencies-jetson/desktop)
RUN sudo apt-get update --fix-missing && \
    /ros_entrypoint.sh rosdep update && \
    /ros_entrypoint.sh rosdep install -i --from-paths src/ -y -r && \
    sudo rm -rf /var/lib/apt/lists/*

## Stage 3: Source Code Copy and Compilation (Repos & Tester Stages)

# Copy all common source repositories
COPY perse_brain src/perse_brain/
COPY perse_sumo src/perse_sumo/

# Setup entrypoint script
COPY perse_entrypoint.sh /
RUN sudo chmod +x /perse_entrypoint.sh; sudo chown ${USERNAME} /perse_entrypoint.sh

# Compile project, install it, run tests and remove unnecessary files
RUN /ros_entrypoint.sh colcon build && \
    sudo rm -rf src log build src

## Stage 4: Final Configuration (Runner Stage)

# Change default shell to fish
RUN sudo chsh -s /usr/bin/fish

# Run project
ENTRYPOINT [ "/perse_entrypoint.sh" ]
