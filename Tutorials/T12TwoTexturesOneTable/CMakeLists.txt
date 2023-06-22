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
set(T12TwoTexturesOneTable_SHADER_SOURCE "./shaders/TwoTextures.hlsl")
source_group("Shaders" FILES ${T12TwoTexturesOneTable_SHADER_SOURCE})
set_source_files_properties(${T12TwoTexturesOneTable_SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

# Source files
set(T12TwoTexturesOneTable_PROJECT_SOURCE  "./src/TwoTexturesOneTableApp.cpp" )

add_executable(T12TwoTexturesOneTable ${T12TwoTexturesOneTable_PROJECT_SOURCE} ${T12TwoTexturesOneTable_SHADER_SOURCE})

target_link_libraries(T12TwoTexturesOneTable PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T12TwoTexturesOneTable includes are private, it uses absolute paths.
set(T12TwoTexturesOneTable_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T12TwoTexturesOneTable PRIVATE "${T12TwoTexturesOneTable_INCLUDE_DIR}")

# Find dependencies:
set(T12TwoTexturesOneTable_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T12TwoTexturesOneTable_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T12TwoTexturesOneTable_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T12TwoTexturesOneTable PRIVATE ${T12TwoTexturesOneTable_LINKED_LIBRARIES})

