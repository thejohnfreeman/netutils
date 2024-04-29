cmake_minimum_required(VERSION 3.16)

project(netutils
  VERSION 0.1.0
  LANGUAGES C CXX
)

find_package(cupcake.cmake REQUIRED)
cupcake_project()
cupcake_find_packages(main)
cupcake_link_libraries(${PROJECT_NAME}.imports.main INTERFACE main)
cupcake_add_libraries()
cupcake_add_executables()
cupcake_install_project()
cupcake_install_cpp_info()
