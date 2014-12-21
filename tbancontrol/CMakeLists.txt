include_directories(../libtban)

add_executable(tbancontrol tbancontrol.c)
target_link_libraries(tbancontrol tban)

install(TARGETS tbancontrol
  DESTINATION ${BIN_INSTALL_DIR})
