cmake_minimum_required(VERSION 3.15)
project(openglcontext C CXX)

set(CMAKE_CXX_STANDARD 11)
set(base_dir ${CMAKE_CURRENT_LIST_DIR}/../)
set(ext_dir ${CMAKE_CURRENT_LIST_DIR}/../extern)
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/../src)
set(inc_dir ${CMAKE_CURRENT_LIST_DIR}/../include)

include_directories(
  ${inc_dir}
  ${src_dir}
  ${ext_dir}/glad/include
  )

list(APPEND poly_libs
  opengl32.lib
  )

list(APPEND poly_sources
  ${ext_dir}/glad/src/glad.c
  )

add_library(poly STATIC ${poly_sources})

list(APPEND poly_deps
  poly
  )

message(${poly_sources})
  
macro(create_test name)

  set(test_name "test-${name}${debug_flag}")
  add_executable(${test_name} WIN32 ${src_dir}/test-${name}.cpp)
  add_dependencies(${test_name} ${poly_deps})
  target_link_libraries(${test_name} ${poly_libs})
  install(TARGETS ${test_name} DESTINATION bin/)

  # Create an a win32 app that uses `main()` instead of `WinMain` with a console 
  set_target_properties(${test_name} PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup"
    )
endmacro()

#create_test("create-gl-context")
create_test("basic-window")
create_test("shared-context")
create_test("shared-context-threading")


