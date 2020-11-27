cmake_minimum_required(VERSION 3.8)
project(bhsim LANGUAGES C)

if(WIN32)
else()
	set(EXTRA_LIBS m)
endif()

set(SIM_TARGET sim)
add_executable(${SIM_TARGET}
    src/sim.c
	src/csv.c
    src/stretchy_buffer.h
    )

set_target_properties(${SIM_TARGET} PROPERTIES C_STANDARD 90)
set_target_properties(${SIM_TARGET} PROPERTIES C_EXTENSIONS OFF)

target_link_libraries(${SIM_TARGET} PRIVATE ${EXTRA_LIBS})


if(NOT NO_VIEWER)
	set(VIEWER_TARGET viewer)
	set(VIEWER_SOURCES
		src/viewer.c
		src/csv.c
		src/linear.c
		${CMAKE_SOURCE_DIR}/extern/glad/src/glad.c
		)

	if(WIN32)
        set(SDL_LIBRARY "${CMAKE_SOURCE_DIR}/extern/SDL2-2.0.12/lib/x64/SDL2.lib;${CMAKE_SOURCE_DIR}/extern/SDL2-2.0.12/lib/x64/SDL2main.lib")
        set(SDL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/SDL2-2.0.12/include)
	else(WIN32)
	endif(WIN32)

	add_executable(${VIEWER_TARGET} ${VIEWER_SOURCES})

	set_target_properties(${VIEWER_TARGET} PROPERTIES C_STANDARD 90)
	set_target_properties(${VIEWER_TARGET} PROPERTIES C_EXTENSIONS OFF)

    target_link_libraries(${VIEWER_TARGET} PRIVATE ${EXTRA_LIBS} ${SDL_LIBRARY})
    target_include_directories(${VIEWER_TARGET} PRIVATE
        ${SDL_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}/extern/glad/include
        )
endif()
