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
set(T00GLM_PROJECT_SOURCE  "./src/GLMExample.cpp" )

add_executable(T00GLM ${T00GLM_PROJECT_SOURCE} ${T00GLM_SHADER_SOURCE})

target_link_libraries(T00GLM PRIVATE project_options project_warnings) # link project_options/warnings

# Includes
# because T00GLM includes are private, it uses absolute paths.
set(T00GLM_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE STRING "")
target_include_directories(T00GLM PRIVATE "${T00GLM_INCLUDE_DIR}")

# Find dependencies:
set(T00GLM_DEPENDENCIES_CONFIGURED    
    glm    
    CACHE STRING "")

foreach(DEPENDENCY ${T00GLM_DEPENDENCIES_CONFIGURED})
  find_package(${DEPENDENCY} CONFIG REQUIRED)
endforeach()

# Link dependencies
set(T00GLM_LINKED_LIBRARIES gimslib glm::glm)
target_link_system_libraries(T00GLM PRIVATE ${T00GLM_LINKED_LIBRARIES})

