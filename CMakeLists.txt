cmake_minimum_required(VERSION 3.18)
project(GLSugar)

set (STB_PATH "" CACHE PATH "Path to stb headers")

if (STB_PATH)
add_definitions(-DSTB_BUILD)
message("GLSugar : Adding stb image loaders")
endif()

#preclude having to do the stb style nonsense
add_library(GLSugar STATIC GLSugar.cpp)
target_include_directories(GLSugar PUBLIC ./)
target_include_directories(GLSugar PUBLIC ${STB_PATH})
target_link_libraries(GLSugar ImGui)
target_link_libraries(GLSugar glhpp)
target_link_libraries(GLSugar glad)

