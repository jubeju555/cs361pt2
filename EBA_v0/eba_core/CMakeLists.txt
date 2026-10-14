add_library(eba_bufreq
   SHARED
      bufreq.c
)

target_include_directories(eba_bufreq
   PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/bufreq_include
)

set_target_properties(eba_bufreq
   PROPERTIES
      OUTPUT_NAME "bufreq"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(eba_bufreq
   PRIVATE
      eba_common
      ${CMAKE_DL_LIBS}
)





add_library(eba_core
   INTERFACE
)

target_link_libraries(eba_core
   INTERFACE
      eba_common
      eba_bufreq
      ${CMAKE_DL_LIBS}
)
