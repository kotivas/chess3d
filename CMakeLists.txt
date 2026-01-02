cmake_minimum_required(VERSION 3.20)
project(chess3d C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp REQUIRED)
find_package(rapidjson REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")
endif ()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCE
        src/main.cpp
        src/Common/Utils.cpp
        src/Common/GlUtils.cpp
        src/Common/Config.cpp
        src/Renderer/Shader.cpp
        src/Renderer/Renderer.cpp
        src/Renderer/Model.cpp
        src/Renderer/PostEffects/GaussianBlur.cpp
        src/Game/Game.cpp
        src/Game/CameraController.cpp
        src/Input/Input.cpp
        src/UI/Console/Console.cpp
        src/UI/Text/MSDFText.cpp
        src/ResourceMgr/ResourceMgr.cpp
        src/ResourceMgr/ResourceMgrModel.cpp
        src/Core/Logger.cpp
        src/Core/CMDUtils.cpp
        src/Core/Backend.cpp
        src/AssetManager/ShaderManager.cpp
        src/Renderer/RenderPasses/ShadowPass.cpp
        src/Renderer/RenderPasses/LightningPass.cpp
        src/Renderer/RenderPasses/UiPass.cpp
        src/Renderer/Sky.cpp
)

add_executable(chess3d ${SOURCE})

target_include_directories(chess3d PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(chess3d PRIVATE OpenGL::GL glad::glad glfw glm::glm assimp::assimp rapidjson)

