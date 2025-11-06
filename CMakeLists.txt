cmake_minimum_required (VERSION 3.14)
project(chess3d C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp REQUIRED)
find_package(rapidjson REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCE 
		src/main.cpp
		src/game/camera.cpp
		src/render/shader.cpp
		src/render/renderer.cpp
		src/render/model.cpp
		src/game/game.cpp
 		src/resourcemgr/resourcemgr.cpp
		src/com/util.cpp
		src/render/shadow.cpp
		src/com/config.cpp
		src/input/input.cpp
		src/console/console.cpp
		src/text/MSDFText.cpp
		src/resourcemgr/resourcemgr_model.cpp
		src/resourcemgr/resourcemgr_shader.cpp
		src/core/logger.cpp
)

add_executable(chess3d ${SOURCE} )
add_custom_command(
	TARGET chess3d POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/assets"
	"${CMAKE_BINARY_DIR}/assets" )

add_custom_command(
	TARGET chess3d POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/src/render/shaders"
	"${CMAKE_BINARY_DIR}/shaders" )

target_include_directories(chess3d PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(chess3d PRIVATE OpenGL::GL glad::glad glfw glm::glm imgui::imgui assimp::assimp rapidjson)

