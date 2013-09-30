#tk.cmake
set(tk_prefix lib/tk/)

# Build TK env
exec_program(${CMAKE_COMMAND} ${tk_prefix}
  ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DDISTRIBUTION=${DISTRIBUTION} 
)
# Creation of tk targets
add_custom_target(libtk 
  COMMAND make -C ${tk_prefix}
)
add_custom_target(libtk_clean-all 
  COMMAND make -C ${tk_prefix} clean-all
)

add_dependencies(nec libtk)
add_dependencies(clean-all libtk_clean-all)

set(extra_clean 
  ${tk_prefix}lib/${DISTRIBUTION}/${CMAKE_BUILD_TYPE}/libtk.so 
)
file(GLOB_RECURSE objects "${tk_prefix}src/tk/*.o")
foreach(file ${objects})
  set(extra_clean ${extra_clean} ${file})
endforeach()
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean}")

