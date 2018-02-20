cmake_minimum_required(VERSION 3.5.1)
project (Simple_Obj_Converter)

# set release or debug builds
if(CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_CONFIGURATION_TYPES Release RelWithDebInfo Debug)
   set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
     "Reset the configurations to what we need"
     FORCE)
 endif()
#SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
# for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/bin )
		set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/lib )
		set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# set flags for debug and release mode
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-g HAS_DEBUG)
check_cxx_compiler_flag(-Wall HAS_WALL)
check_cxx_compiler_flag(-O0 HAS_OPT0)
check_cxx_compiler_flag(-O3 HAS_OPT3)
check_cxx_compiler_flag(-fmessage-length=0 HAS_MSG)
check_cxx_compiler_flag(-std=c++11 HAS_CNEW)
check_cxx_compiler_flag(-lstdc++fs HAS_FS)
if(HAS_DEBUG)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
endif()
if(HAS_WALL)
	if(UNIX)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
	else()
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W4")
	endif()
endif()
if(HAS_OPT0)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
endif()
if(HAS_OPT3)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()
if(HAS_MSG)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")
endif()

# use the file(GLOB...) or file(GLOB_RECURSE...) to grab wildcard src files
file(GLOB_RECURSE SOURCES 	"${CMAKE_SOURCE_DIR}/src/*.cpp")

if (MSVC)
	# warning that pop up for strtok & fopen & std::copy
	add_definitions("-D_SCL_SECURE_NO_WARNINGS")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

# clang-format
if (UNIX)
	add_custom_target(
		clangformat
		COMMAND /usr/bin/clang-format
		-style=google
		-i
		${SOURCES} ${PROJECT_SOURCES}
	)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(obj_to_ddm ${SOURCES})

# set visual studio startup project
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
			obj_to_ddm)