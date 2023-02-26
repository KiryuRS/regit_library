function(add_regit_tests filename_without_ext)

  add_executable(regit_${filename_without_ext}
    ${filename_without_ext}.cpp)

  target_include_directories(regit_${filename_without_ext}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/..)

endfunction()
