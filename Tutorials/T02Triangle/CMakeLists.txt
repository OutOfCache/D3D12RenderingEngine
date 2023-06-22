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
set(T02Triangle_SHADER_SOURCE "./shaders/Triangle.hlsl")
source_group("Shaders" FILES ${T02Triangle_SHADER_SOURCE})
set_source_files_properties(${T02Triangle_SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

# Source files
set(T02Triangle_PROJECT_SOURCE  "./src/TriangleApp.cpp" )

add_executable(T02Triangle ${T02Triangle_PROJECT_SOURCE} ${T02Triangle_SHADER_SOURCE})

target_link_libraries(T02Triangle PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T02Triangle includes are private, it uses absolute paths.
set(T02Triangle_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T02Triangle PRIVATE "${T02Triangle_INCLUDE_DIR}")

# Find dependencies:
set(T02Triangle_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T02Triangle_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T02Triangle_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T02Triangle PRIVATE ${T02Triangle_LINKED_LIBRARIES})

