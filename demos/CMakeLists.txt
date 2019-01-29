# basic demonstration program
ADD_EXECUTABLE(basic_demo basic_demo.c)
TARGET_LINK_LIBRARIES(basic_demo libyamlConfigFile)

# What does "make install" do?
INSTALL(TARGETS basic_demo
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
