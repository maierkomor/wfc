cmake_minimum_required(VERSION 3.8.0)

# version determination

find_package(Hg)


# flex/bison

find_package(BISON 3.0 REQUIRED)
find_package(FLEX 2.6 REQUIRED)

BISON_TARGET(xproto_y
	src/xproto.y
	src/xproto.cc
	#${CMAKE_CURRENT_BINARY_DIR}/xproto_y.cpp
	COMPILE_FLAGS "-Lc++ --defines=xproto.h"
	DEFINES_FILE src/xproto.h
)

FLEX_TARGET(xproto_l
	src/xproto.l
	${CMAKE_CURRENT_BINARY_DIR}/xproto_l.cpp
	COMPILE_FLAGS "-F -R --bison-locations"
)

ADD_FLEX_BISON_DEPENDENCY(xproto_l xproto_y)


# generic stuff

set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -Isrc")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CAMKE_CURRENT_BINARY_DIR} src)


add_custom_target(
	version_h
	COMMAND bash mkversion.sh
	OUTPUT src/version.h
	WORKING_DIRECTORY src
)

add_custom_command(
	OUTPUT src/template_lib.cc
	COMMAND bash genlib.sh
	WORKING_DIRECTORY src
)

add_executable(wfc
	src/CodeGenerator.cc
	src/CodeLibrary.cc
	src/CodeTemplate.cc
	src/CppGenerator.cc
	src/Decoder.cc
	src/Enum.cc
	src/Evaluator.cc
	src/Field.cc
	src/FoldCompounds.cc
	src/Generator.cc
	src/Indenter.cc
	src/io.cc
	src/keywords.cc
	src/log.cc
	src/Message.cc
	src/Options.cc
	src/PBFile.cc
	src/ProtoDriver.cc
	src/template_lib.cc
	src/wfc.cc
	src/wirefuncs.cc
	src/XmlGenerator.cc
	${BISON_xproto_y_OUTPUTS}
	${FLEX_xproto_l_OUTPUTS}
)

target_compile_features(wfc
	PRIVATE cxx_std_11
)

add_dependencies(wfc
	version_h
)

file(GLOB SHAREDFILES
	share/*.cc
	share/*.cct
)

file(GLOB HEADERS
	include/*.h
)

install(
	FILES bin/wfc
	DESTINATION bin
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
) 

install(
	FILES doc/wfc.1
	DESTINATION man/man1
)

install(
	FILES ${HEADERS}
	DESTINATION include
)

install(
	FILES ${SHAREDFILES}
	DESTINATION share
)
