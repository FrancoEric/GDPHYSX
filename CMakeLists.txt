# CMakeList.txt : CMake project for gdphysx, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("gdphysx" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glm 
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
)

FetchContent_MakeAvailable(glfw)

add_library(glad STATIC
    external/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

find_package(OpenGL REQUIRED)

target_link_libraries(glad PUBLIC OpenGL::GL)

add_executable (gdphysx "gdphysx.cpp"  "tiny_obj_loader.h" "object.h")

target_link_libraries(gdphysx
    PRIVATE
        glfw
        glad
        glm::glm
)

add_custom_command(TARGET gdphysx 
				   POST_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
					"${CMAKE_CURRENT_SOURCE_DIR}/3D"
					"$<TARGET_FILE_DIR:gdphysx>/3D"
					COMMENT "copying assets"
)

add_custom_command(TARGET gdphysx 
				   POST_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
					"${CMAKE_CURRENT_SOURCE_DIR}/shaders"
					"$<TARGET_FILE_DIR:gdphysx>/shaders"
					COMMENT "copying shaders"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gdphysx PROPERTY CXX_STANDARD 20)
endif()

if (WIN32)
    target_link_libraries(gdphysx PRIVATE opengl32)
endif()