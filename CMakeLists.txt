cmake_minimum_required(VERSION 3.5.0)
project(QoalEngine VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_definitions(-DGLFW_INCLUDE_VULKAN)

include_directories(SYSTEM
  "C:/VulkanSDK/1.3.261.1/Include"
  "C:/glfw-3.4/include"
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "/vkr/renderers/shaders/GLSL/*.frag"
    "/vkr/renderers/shaders/GLSL/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "/vkr/renderers/shaders/SPIR-V/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND glslc ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    
endforeach(GLSL &{GLSL_SOURCE_FILES})

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

add_executable(QoalEngine main.cpp ${SPIRV_BINARY_FILES})

target_include_directories(QoalEngine PRIVATE 
    "../Qarbon/src"
    "../Qandle/src"
)

target_link_libraries(QoalEngine PRIVATE Vulkan::Vulkan glfw)

add_dependencies(QoalEngine Shaders)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)