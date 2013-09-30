# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "nec_config.h.in"
  "nec_config.h"
)


set(bin_src 
  ${NEC_SRC_DIR}/nec.c
)
add_executable(nec ${bin_src})
target_link_libraries(nec tk)
