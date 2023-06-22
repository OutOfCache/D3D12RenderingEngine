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
set(T04Ellipsis_SHADER_SOURCE "./shaders/EllipsisApp.hlsl")
source_group("Shaders" FILES ${T04Ellipsis_SHADER_SOURCE})
set_source_files_properties(${T04Ellipsis_SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

# Source files
set(T04Ellipsis_PROJECT_SOURCE  "./src/EllipsisApp.cpp" )

add_executable(T04Ellipsis ${T04Ellipsis_PROJECT_SOURCE} ${T04Ellipsis_SHADER_SOURCE})

target_link_libraries(T04Ellipsis PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T04Ellipsis includes are private, it uses absolute paths.
set(T04Ellipsis_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T04Ellipsis PRIVATE "${T04Ellipsis_INCLUDE_DIR}")

# Find dependencies:
set(T04Ellipsis_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T04Ellipsis_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T04Ellipsis_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T04Ellipsis PRIVATE ${T04Ellipsis_LINKED_LIBRARIES})

