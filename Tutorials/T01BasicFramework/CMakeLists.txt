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

# Source files
set(T01BasicFramework_PROJECT_SOURCE  "./src/BasicFrameWork.cpp" )

add_executable(T01BasicFramework ${T01BasicFramework_PROJECT_SOURCE} ${T01BasicFramework_SHADER_SOURCE})

target_link_libraries(T01BasicFramework PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T01BasicFramework includes are private, it uses absolute paths.
set(T01BasicFramework_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T01BasicFramework PRIVATE "${T01BasicFramework_INCLUDE_DIR}")

# Find dependencies:
set(T01BasicFramework_DEPENDENCIES_CONFIGURED    
    glm
    CACHE STRING "")

foreach(DEPENDENCY ${T01BasicFramework_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T01BasicFramework_LINKED_LIBRARIES glm::glm gimslib "d3d12.lib" "dxgi.lib")
target_link_system_libraries(T01BasicFramework PRIVATE ${T01BasicFramework_LINKED_LIBRARIES})

