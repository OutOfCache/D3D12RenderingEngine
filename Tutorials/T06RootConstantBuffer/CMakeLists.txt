# Defintions
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
add_definitions(-DNOMINMAX)
add_definitions(-DNODRAWTEXT)
add_definitions(-DNOBITMAP)
add_definitions(-DNOMCX)
add_definitions(-DNOSERVICE)
add_definitions(-DNOHELP)
add_definitions(-DWIN32_LEAN_AND_MEAN)

#Shader files
set(T06RootConstantBuffer_SHADER_SOURCE "./shaders/RootConstantBuffer.hlsl")
source_group("Shaders" FILES ${T06RootConstantBuffer_SHADER_SOURCE})
set_source_files_properties(${T06RootConstantBuffer_SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

# Source files
set(T06RootConstantBuffer_PROJECT_SOURCE  "./src/RootConstantBufferApp.cpp" )

add_executable(T06RootConstantBuffer ${T06RootConstantBuffer_PROJECT_SOURCE} ${T06RootConstantBuffer_SHADER_SOURCE})

target_link_libraries(T06RootConstantBuffer PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T06RootConstantBuffer includes are private, it uses absolute paths.
set(T06RootConstantBuffer_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T06RootConstantBuffer PRIVATE "${T06RootConstantBuffer_INCLUDE_DIR}")

# Find dependencies:
set(T06RootConstantBuffer_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T06RootConstantBuffer_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T06RootConstantBuffer_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T06RootConstantBuffer PRIVATE ${T06RootConstantBuffer_LINKED_LIBRARIES})

