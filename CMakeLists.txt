cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(VoxelGame)

set(sources
	"src/VoxelGame.cpp"
	"src/Log.cpp"
	"src/Utility.cpp"
	"src/JobSystem.cpp"
	"src/Renderer.cpp"
	"src/Shader.cpp"
	"src/VertexBuffer.cpp"
	"src/Texture.cpp"
	"src/TextureArray.cpp"
	"src/FreeCamera.cpp"
	"vendor/glad/src/glad.c"
	"vendor/stb_image/src/stb_image.cpp"
)

if(WIN32)
	link_directories("vendor/SDL2/lib/x64")
endif()

add_executable(VoxelGame ${sources})

if(UNIX)
	target_include_directories(VoxelGame PRIVATE "/usr/include/SDL2")
	target_link_libraries(VoxelGame "dl" "pthread")
endif()

if(WIN32)
	target_include_directories(VoxelGame PRIVATE "vendor/SDL2/include")
	target_link_libraries(VoxelGame "OpenGL32" "SDL2main")

	add_custom_command(TARGET VoxelGame POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
			"${CMAKE_CURRENT_LIST_DIR}/vendor/SDL2/lib/x64/SDL2.dll"
			$<TARGET_FILE_DIR:VoxelGame>)
endif()

if(ARB_DIRECT_STATE_ACCESS)
	target_compile_definitions(VoxelGame PRIVATE ARB_DIRECT_STATE_ACCESS)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(VoxelGame PRIVATE DEBUG)
endif()

target_include_directories(VoxelGame PRIVATE "vendor/glad/include" "vendor/glm" "vendor/stb_image/include")
target_link_libraries(VoxelGame "SDL2")
set_target_properties(VoxelGame PROPERTIES CXX_STANDARD 17)