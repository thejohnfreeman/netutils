cmake_minimum_required(VERSION 3.16)

project(netutils
  VERSION 0.1.0
  LANGUAGES C CXX
)

find_package(cupcake 0.4.0 REQUIRED)

cupcake_project()

# imports
target_link_libraries(netutils.dependencies PUBLIC
)

cupcake_add_library(jfnet)
target_link_libraries(${this} PUBLIC netutils.dependencies)

cupcake_add_executable(ping)
target_link_libraries(${this} PRIVATE netutils.libjfnet m)

cupcake_add_executable(traceroute)
target_link_libraries(${this} PRIVATE netutils.libjfnet)

cupcake_add_executable(ip2geo)
target_link_libraries(${this} PRIVATE netutils.libjfnet)

cupcake_install_project()
cupcake_install_cpp_info()
