set(TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)
file(MAKE_DIRECTORY ${TMPDIR})

function(gs_add_test name)
  add_executable(test_${name} ${name}.c)
  target_link_libraries(test_${name} PRIVATE GS::gs)
  target_compile_features(test_${name} PRIVATE c_std_11)
  target_compile_options(test_${name} PRIVATE ${WARNING_FLAGS})
  target_compile_definitions(test_${name} PUBLIC "TMPDIR=\"${TMPDIR}\"")
  add_test(NAME ${name} COMMAND test_${name})
endfunction()

gs_add_test(gs)
