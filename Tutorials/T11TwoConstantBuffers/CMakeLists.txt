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
set(T11TwoConstantBuffers_SHADER_SOURCE "./shaders/TwoConstantBuffers.hlsl")
source_group("Shaders" FILES ${T11TwoConstantBuffers_SHADER_SOURCE})
set_source_files_properties(${T11TwoConstantBuffers_SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

# Source files
set(T11TwoConstantBuffers_PROJECT_SOURCE  "./src/TwoConstantBuffersApp.cpp" )

add_executable(T11TwoConstantBuffers ${T11TwoConstantBuffers_PROJECT_SOURCE} ${T11TwoConstantBuffers_SHADER_SOURCE})

target_link_libraries(T11TwoConstantBuffers PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T11TwoConstantBuffers includes are private, it uses absolute paths.
set(T11TwoConstantBuffers_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T11TwoConstantBuffers PRIVATE "${T11TwoConstantBuffers_INCLUDE_DIR}")

# Find dependencies:
set(T11TwoConstantBuffers_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T11TwoConstantBuffers_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T11TwoConstantBuffers_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T11TwoConstantBuffers PRIVATE ${T11TwoConstantBuffers_LINKED_LIBRARIES})

