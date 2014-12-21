add_library(tban SHARED tban.c mini_ng.c parser.c big_ng.c)


install(FILES tban.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libtban COMPONENT Devel)

install(TARGETS tban
  DESTINATION ${LIB_INSTALL_DIR})
