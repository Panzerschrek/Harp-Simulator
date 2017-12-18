cmake_minimum_required(VERSION 2.8)

project( HarpSimulator )

set( CMAKE_CXX_STANDARD 11 )

# Set path to your SDL2 here.
if( WIN32 )
	set( SDL_BASE_DIR ../SDL2-2.0.3 )
	set( SDL_INCLUDES_DIR ${SDL_BASE_DIR}/include )
	set( SDL_LIBS_DIR ${SDL_BASE_DIR}/lib/x86 )
	get_filename_component( SDL_LIBS_DIR_ABSOLUTE ${SDL_LIBS_DIR} ABSOLUTE )

	set( SDL_TTF_BASE_DIR ../SDL2_ttf-2.0.14 )
	set( SDL_TTF_INCLUDES_DIR ${SDL_TTF_BASE_DIR}/include )
	set( SDL_TTF_LIBS_DIR ${SDL_TTF_BASE_DIR}/lib/x86 )
	get_filename_component( SDL_TTF_LIBS_DIR_ABSOLUTE ${SDL_TTF_LIBS_DIR} ABSOLUTE )
else()
	set( SDL_INCLUDES_DIR /usr/include/SDL2 )
	set( SDL_TTF_INCLUDES_DIR /usr/include/SDL2 )
endif()

include_directories( ${SDL_INCLUDES_DIR} )
include_directories( ${SDL_TTF_INCLUDES_DIR} )

if( WIN32 )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows" ) # force gui, not console application
endif()

add_executable( HarpSimulator main.cpp )

if( WIN32 )
	target_link_libraries( HarpSimulator ${SDL_LIBS_DIR_ABSOLUTE}/SDL2main.lib )
	target_link_libraries( HarpSimulator ${SDL_LIBS_DIR_ABSOLUTE}/SDL2.lib )
	target_link_libraries( HarpSimulator ${SDL_TTF_LIBS_DIR_ABSOLUTE}/SDL2_ttf.lib )
else()
	target_link_libraries( HarpSimulator SDL2 )
	target_link_libraries( HarpSimulator SDL2_ttf )
endif()
