cmake_minimum_required(VERSION 3.8)
project(perse_brain)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Encontra as dependências
find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

ament_export_dependencies(rclcpp)

# Cria o binário "brain" utilizando o arquivo brain.cpp
add_executable(brain
  src/brain.cpp
)
# atribui as dependências
ament_target_dependencies(brain
  rclcpp
  sensor_msgs
  geometry_msgs
)

# Instala o executável
install(TARGETS
  brain

  DESTINATION lib/${PROJECT_NAME}
)
ament_export_libraries(brain)

# Instala os arquivos não executáveis (configuração e launch)
foreach(DIR launch config)
  install(DIRECTORY ${DIR}
    DESTINATION share/${PROJECT_NAME}
  )
endforeach()

ament_package()
