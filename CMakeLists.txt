cmake_minimum_required (VERSION 3.13)

include(FetchContent)
project ("BromSock" VERSION 1.0.0)

set(GARRYSMOD_FOLDER "" CACHE FILEPATH "optional garrysmod path to deploy to on post-build, eg: 'D:/SteamLibrary/steamapps/common/GarrysMod'")

# flags
set(build_tests FALSE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_EXTERNAL -DNOMINMAX")

# for non-SSL mode
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBROMSOCK_NOSSL")

# The newest garrysmod headers doesn't seem to compile at all without the old stuff.....?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGMOD_ALLOW_DEPRECATED")

# conan for libaries
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

if(CMAKE_COMPILER_IS_GNUXX)
   set(CONAN_SETTINGS compiler.libcxx=libstdc++11)
endif()

conan_cmake_run(CONANFILE conanfile.txt
                SETTINGS ${CONAN_SETTINGS}
                BUILD missing)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

FetchContent_GetProperties(mainframe)
if(NOT mainframe_POPULATED)
	FetchContent_Declare(mainframe
		GIT_REPOSITORY https://github.com/Goofy-Penguin/Mainframe.git
		GIT_TAG origin/master
	)

	FetchContent_MakeAvailable(mainframe)
endif()

FetchContent_GetProperties(gmod-module-base)
if(NOT gmod-module-base_POPULATED)
	FetchContent_Declare(gmod-module-base
		GIT_REPOSITORY https://github.com/Facepunch/gmod-module-base.git
		GIT_TAG origin/development
	)

	FetchContent_MakeAvailable(gmod-module-base)
endif()

find_package(nlohmann_json)
find_package(fmt)
find_package(OpenSSL)

add_subdirectory(gmod)
add_subdirectory(bromsock)
add_subdirectory(tester)

# These targets do not seem to be able to build...?
set_target_properties(MetaTable HelloWorld PROPERTIES
   EXCLUDE_FROM_ALL 1
   EXCLUDE_FROM_DEFAULT_BUILD 1
)
