cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_SKIP_BUILD_RPATH true)
project(sample CXX)

pkg_check_modules(pkgs REQUIRED
	egl
	glesv2
	wayland-server
	wayland-egl
	libinput
	libudev
	libdrm
	gbm
)

foreach(CFLAG ${pkgs_CFLAGS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAG}")
endforeach(CFLAG)

MESSAGE(${pkgs_LDFLAGS})
aux_source_directory(src SRCS)
include_directories(include)
add_executable(ncway-${PROJECT_NAME} ${SRCS}) 
target_link_libraries(ncway-${PROJECT_NAME} ${pkgs_LDFLAGS})
install(TARGETS ncway-${PROJECT_NAME} DESTINATION bin)

