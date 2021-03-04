cmake_minimum_required(VERSION 3.1)
project(larcanoid)

set(HEADER_FILES
	"Sources/Actor.h"
	"Sources/Arcanoid.h"
	"Sources/Config.h"
	"Sources/Engine.h"
	"Sources/FMath.h"
	"Sources/Timer.h"
)

set(SOURCE_FILES
	"Sources/Arcanoid.cpp"
	"Sources/Engine.cpp"
	"Sources/Timer.cpp"
	"Sources/main.cpp"
)

add_executable (larcanoid ${HEADER_FILES} ${SOURCE_FILES})

set_property(TARGET larcanoid PROPERTY CXX_STANDARD 17)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(larcanoid PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(sdl2-image CONFIG REQUIRED)
target_link_libraries(larcanoid PRIVATE SDL2::SDL2_image)

find_package(sdl2-ttf CONFIG REQUIRED)
target_link_libraries(larcanoid PRIVATE SDL2::SDL2_ttf)

find_package(sdl2-mixer CONFIG REQUIRED)
target_link_libraries(larcanoid PRIVATE SDL2::SDL2_mixer)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(larcanoid PRIVATE EnTT::EnTT)
