cmake_minimum_required (VERSION 3.8)
project(chess3d C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Opengl REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(Stb REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(assimp REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")
endif()

set(SOURCE 
	"src/main.cpp"
	"src/game/camera.cpp"
	"src/render/shader.cpp"
	"src/render/renderer.cpp"
	"src/render/model.cpp"
	"src/game/game.cpp"
 "src/game/resource_manager.cpp" "src/util.cpp")

add_executable(chess3d ${SOURCE} )
add_custom_command(
	TARGET chess3d POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/assets"
	"${CMAKE_BINARY_DIR}/assets" )

target_include_directories(chess3d PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(chess3d PRIVATE OpenGL::GL glad::glad glfw glm::glm imgui::imgui tinyobjloader::tinyobjloader assimp::assimp)

