file(GLOB_RECURSE source_files
	"sources/*.cpp"
	"sources/*.c"
	"include/*.h"
	"include/*.hpp"
)

set(output_target bromsock_test)
add_library(${output_target} ${source_files})
target_include_directories(${output_target} PUBLIC
	"include"
)
target_compile_features(${output_target} PRIVATE cxx_std_17)
target_link_libraries(${output_target} PRIVATE bromsock)
set_target_properties(${output_target} PROPERTIES LINK_FLAGS "/ignore:4099")

if(WIN32)
  #string(REGEX REPLACE "/W[1|2|3|4]" "/w4 " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options("/EHsc")

  target_compile_options(${output_target} PUBLIC /wd4100 /wd4189 /wd4458 /wd4459)
else()
  target_compile_options(${output_target} PRIVATE -Wall -Wextra -Wno-pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-reorder -Wno-unknown-pragmas)
endif()
