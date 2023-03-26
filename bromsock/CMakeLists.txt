file(GLOB_RECURSE source_files
	"sources/*.cpp"
	"sources/*.c"
	"include/*.h"
	"include/*.hpp"
)

set(output_target bromsock)
add_library(${output_target} SHARED ${source_files})
target_include_directories(${output_target} PUBLIC
	"include"
)
target_compile_features(${output_target} PRIVATE cxx_std_17)
target_link_libraries(${output_target} PRIVATE
	gmod-module-base
	nlohmann_json::nlohmann_json
	fmt::fmt
	OpenSSL::OpenSSL
	mainframe.math
	mainframe.utils
	mainframe.networking
)


if(WIN32)
  #string(REGEX REPLACE "/W[1|2|3|4]" "/w4 " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options("/EHsc")

  set_target_properties(${output_target} PROPERTIES LINK_FLAGS "/ignore:4099")
  target_compile_options(${output_target} PUBLIC /wd4100 /wd4189 /wd4458 /wd4459)
else()
  target_compile_options(mainframe.networking PUBLIC -fPIC)
  target_compile_options(${output_target} PUBLIC -Wall -Wextra -Wno-pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-reorder -Wno-unknown-pragmas)
endif()

set_gmod_suffix_prefix_better(${output_target})

message("bin sv: ${${output_target}_OUTPUT_BIN_SV}")
message("bin cl: ${${output_target}_OUTPUT_BIN_CL}")
if (GARRYSMOD_FOLDER STREQUAL "")
	message("Skipping deploy to garrysmod folder")
else()
	message("Deploying to garrysmod folder: ${GARRYSMOD_FOLDER}/garrysmod/lua/bin")

	add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${GARRYSMOD_FOLDER}/garrysmod/lua/bin")
	add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${${output_target}_OUTPUT_BIN_SV}" "${GARRYSMOD_FOLDER}/garrysmod/lua/bin/")
	add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${${output_target}_OUTPUT_BIN_CL}" "${GARRYSMOD_FOLDER}/garrysmod/lua/bin/")
endif()

add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/artifacts")
add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${${output_target}_OUTPUT_BIN_SV}" "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/")
add_custom_command(TARGET ${output_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${${output_target}_OUTPUT_BIN_CL}" "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/")
