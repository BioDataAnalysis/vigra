cmake_minimum_required(VERSION 3.12)

##################################################
#
#     basic initializations
#
##################################################

PROJECT(vigra)

# read the current version from config_version.hxx
file(READ ${vigra_SOURCE_DIR}/include/vigra/config_version.hxx VIGRA_VERSION_FILE)
string(REGEX MATCH "VIGRA_VERSION_MAJOR[ \t\n]+[^ \t\n]+" VIGRA_VERSION_MAJOR ${VIGRA_VERSION_FILE})
string(REGEX REPLACE "VIGRA_VERSION_MAJOR[ \t\n]" "" VIGRA_VERSION_MAJOR ${VIGRA_VERSION_MAJOR})
string(REGEX MATCH "VIGRA_VERSION_MINOR[ \t\n]+[^ \t\n]+" VIGRA_VERSION_MINOR ${VIGRA_VERSION_FILE})
string(REGEX REPLACE "VIGRA_VERSION_MINOR[ \t\n]" "" VIGRA_VERSION_MINOR ${VIGRA_VERSION_MINOR})
string(REGEX MATCH "VIGRA_VERSION_PATCH[ \t\n]+[^ \t\n]+" VIGRA_VERSION_PATCH ${VIGRA_VERSION_FILE})
string(REGEX REPLACE "VIGRA_VERSION_PATCH[ \t\n]" "" VIGRA_VERSION_PATCH ${VIGRA_VERSION_PATCH})

SET(vigra_version_short ${VIGRA_VERSION_MAJOR}.${VIGRA_VERSION_MINOR})
SET(vigra_version ${VIGRA_VERSION_MAJOR}.${VIGRA_VERSION_MINOR}.${VIGRA_VERSION_PATCH})
message(STATUS "Configuring VIGRA version ${vigra_version}")

SET(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${vigra_SOURCE_DIR}/config)

include(VigraSetDefaults)
include(VigraCMakeUtils)
INCLUDE_DIRECTORIES(${vigra_SOURCE_DIR}/include)

if(SUPPRESS_3RD_PARTY_WARNINGS)
    set(SUPPRESS_WARNINGS SYSTEM)
endif()

set(CMAKE_CXX_STANDARD 11 CACHE STRING "The CXX standard used for the project")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(VIGRA_STATIC_LIB)
    SET(LIBTYPE STATIC)
    ADD_DEFINITIONS(-DVIGRA_STATIC_LIB)
ELSE(VIGRA_STATIC_LIB)
    SET(LIBTYPE SHARED)
ENDIF(VIGRA_STATIC_LIB)

IF (MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
ENDIF ()

IF(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    set(SUFFICIENT_TEMPLATE_DEPTH FALSE)
    TRY_COMPILE(SUFFICIENT_TEMPLATE_DEPTH
                ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/config/checkTemplateDepth.cxx
                COMPILE_DEFINITIONS "-DDEPTH=900")

    IF(SUFFICIENT_TEMPLATE_DEPTH)
        MESSAGE(STATUS "Checking template recursion depth: ok")
    ELSE()
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # As of cmake 2.8.10, there is a variable CMAKE_CXX_COMPILER_VERSION.
            # Emulate it by a compiler call if not present.
            IF(NOT CMAKE_CXX_COMPILER_VERSION)
                exec_program(${CMAKE_CXX_COMPILER}
                    ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
                    OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION)
                string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1.\\2"
                      CMAKE_CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
            endif()

            message ("Detected GCC version ${CMAKE_CXX_COMPILER_VERSION}")
            if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.4.0")
                MESSAGE(WARNING "GNU c++ < 4.4 cannot build VIGRANumPy; disabling (found GCC ${CMAKE_CXX_COMPILER_VERSION})")
                SET(WITH_VIGRANUMPY 0)
            elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.5.0")
                # gcc 4.4.x syntax is -ftemplate-depth-900 (not =900)
                SET(CMAKE_CXX_FLAGS "-ftemplate-depth-900 ${CMAKE_CXX_FLAGS}")
                MESSAGE(STATUS "Checking template recursion depth: using -ftemplate-depth-900")
            else()
                # new syntax is =900, not -900
                SET(CMAKE_CXX_FLAGS "-ftemplate-depth=900 ${CMAKE_CXX_FLAGS}")
                MESSAGE(STATUS "Checking template recursion depth: using -ftemplate-depth=900")
            endif()
        else()
            # clang uses newer gcc syntax
            SET(CMAKE_CXX_FLAGS "-ftemplate-depth=900 ${CMAKE_CXX_FLAGS}")
            MESSAGE(STATUS "Checking template recursion depth: using -ftemplate-depth=900")
        endif()
    ENDIF()
ENDIF()

if(MSVC)
    string(REGEX MATCH "[0-9.]+" MSVC_VERSION ${CMAKE_GENERATOR})
endif()

if(MACOSX)
    set(CMAKE_MACOSX_RPATH 1)
endif()

option(BUILD_DOCS "Build documentation" ON)
option(BUILD_TESTS "Build test programs" ON)

##################################################
#
#     search for dependencies
#
##################################################

INCLUDE(VigraFindPackage)
VIGRA_FIND_PACKAGE(ZLIB)
VIGRA_FIND_PACKAGE(TIFF NAMES libtiff_i libtiff) # prefer DLL on Windows
VIGRA_FIND_PACKAGE(JPEG NAMES libjpeg)
VIGRA_FIND_PACKAGE(PNG)
VIGRA_FIND_PACKAGE(FFTW3 NAMES libfftw3-3 libfftw-3.3)
VIGRA_FIND_PACKAGE(FFTW3F NAMES libfftw3f-3 libfftwf-3.3)


IF(WITH_OPENEXR)
    FIND_PACKAGE(Imath CONFIG)
    IF(TARGET Imath::Imath)
        FIND_PACKAGE(OpenEXR CONFIG)
        SET(OPENEXR_LIBRARIES
		    OpenEXR::OpenEXR
		    OpenEXR::Iex
			OpenEXR::IlmThread
			Imath::Imath
		)
		get_target_property(OPENEXR_INCLUDE_DIR OpenEXR::OpenEXRConfig INTERFACE_INCLUDE_DIRECTORIES)
		get_target_property(IMATH_INCLUDE_DIR Imath::ImathConfig INTERFACE_INCLUDE_DIRECTORIES)
		include_directories(${OPENEXR_INCLUDE_DIR} ${IMATH_INCLUDE_DIR})
		set(OPENEXR_FOUND TRUE)
	ELSE()
    VIGRA_FIND_PACKAGE(OpenEXR)
	ENDIF()
ENDIF()

IF(WITH_HDF5)
    VIGRA_FIND_PACKAGE(HDF5)
ENDIF()

IF(WITH_LEMON)
    VIGRA_FIND_PACKAGE(LEMON)
ENDIF()

SET(DOXYGEN_SKIP_DOT TRUE)

IF(BUILD_DOCS)
    FIND_PACKAGE(Doxygen)
    FIND_PACKAGE(Python COMPONENTS Interpreter)
ENDIF()

##################################################
#
#     configure boost dependency
#
##################################################

set(WITH_BOOST "OFF")
set(BOOST_MINIMUM_VERSION "1.40.0")
set(WITH_BOOST_COMPONENTS "")

IF(WITH_BOOST_GRAPH)
    set(WITH_BOOST "ON")
ENDIF()

IF(WITH_VIGRANUMPY)
    set(WITH_BOOST "ON")
ENDIF()

IF(WITH_BOOST_THREAD)
    set(WITH_BOOST "ON")
    set(WITH_BOOST_COMPONENTS ${WITH_BOOST_COMPONENTS} thread system date_time chrono)
    set(BOOST_MINIMUM_VERSION "1.55.0")
ENDIF()

IF(WITH_BOOST)
    find_package(Boost CONFIG ${BOOST_MINIMUM_VERSION} COMPONENTS ${WITH_BOOST_COMPONENTS})
    # FIXME: the following may be unstable between cmake versions
    SET(Boost_LIB_SUFFIX "${_boost_COMPILER}${_boost_MULTITHREADED}-${Boost_LIB_VERSION}")
ENDIF()

if(WITH_BOOST AND Boost_FOUND)
    INCLUDE_DIRECTORIES(${SUPPRESS_WARNINGS} ${Boost_INCLUDE_DIR})
    IF(WITH_BOOST_THREAD)
        ADD_DEFINITIONS(-DUSE_BOOST_THREAD)
    ENDIF()
endif()

# Must be included AFTER boost is found.
include(VigraDetectThreading)
include(VigraConfigureThreading)

IF(WITH_VIGRANUMPY)
    FIND_PACKAGE(Python COMPONENTS Interpreter Development NumPy)
    FIND_PACKAGE(VIGRANUMPY_DEPENDENCIES)
ENDIF()

if(WITH_BOOST AND Boost_FOUND)
    # configure boost's autolink magic to use the right library name
    # (default on Windows is a mangled name like 'boost_system-vc110-mt-1_51.lib')
    if((Boost_SYSTEM_LIBRARY MATCHES "boost_system\\.lib") OR
       (Boost_PYTHON_LIBRARY MATCHES "boost_python\\.lib") OR
       (Boost_PYTHON_LIBRARY MATCHES "boost_python${Python_VERSION_MAJOR}\\.lib"))
        set(Boost_MANGLED_NAMES 0)
        ADD_DEFINITIONS(-DBOOST_AUTO_LINK_NOMANGLE)
    else()
        set(Boost_MANGLED_NAMES 1)
    endif()
    if((Boost_SYSTEM_LIBRARY AND
        NOT Boost_SYSTEM_LIBRARY MATCHES "libboost_system") OR
       (Boost_PYTHON_LIBRARY AND
        NOT Boost_PYTHON_LIBRARY MATCHES "libboost_python"))
        ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
    endif()
endif()

IF(WITH_VALGRIND)
    FIND_PROGRAM(VALGRIND_EXECUTABLE valgrind)
ENDIF()

########################################
#  finalize compiler flags
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}"
    CACHE STRING  "Flags used by the C compiler during all build types" FORCE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}"
    CACHE STRING  "Flags used by the C++ compiler during all build types" FORCE)

##################################################
#
#     setup testing environment
#
##################################################

ADD_CUSTOM_TARGET(check)
ADD_CUSTOM_TARGET(ctest COMMAND ${CMAKE_CTEST_COMMAND})

ADD_CUSTOM_TARGET(check_cpp)
ADD_DEPENDENCIES(check check_cpp)


OPTION(CREATE_CTEST_TARGETS "Create special targets for CTest" OFF)
IF(CREATE_CTEST_TARGETS)
  INCLUDE(CTest)
ENDIF()

IF(WITH_VIGRANUMPY)
    ADD_CUSTOM_TARGET(check_python)
    ADD_DEPENDENCIES(check check_python)
ENDIF()

ENABLE_TESTING()

##################################################
#
#     setup documentation generator
#
##################################################

ADD_CUSTOM_TARGET(doc)

# Automatically push the latest documentation to github's gh-pages.
# This only works when ${DOCDIR} refers to a VIGRA repository that is
# checked out in branch 'gh-pages', and the present repository is
# in branch 'master' (these conditions are checked in the script).
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    configure_file(${PROJECT_SOURCE_DIR}/config/update-gh-pages.sh.in
                   ${PROJECT_BINARY_DIR}/update-gh-pages.sh @ONLY)
    EXECUTE_PROCESS(COMMAND chmod u+x ${PROJECT_BINARY_DIR}/update-gh-pages.sh OUTPUT_QUIET ERROR_QUIET)

    ADD_CUSTOM_TARGET(gh-pages
          ${PROJECT_BINARY_DIR}/update-gh-pages.sh
          DEPENDS doc
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
          COMMENT "Pushing documentation to github pages.")
endif()

##################################################
#
#     setup private experimentation target
#
##################################################

ADD_CUSTOM_TARGET(experiments)

##################################################
#
#     subdirectories
#
##################################################

ADD_SUBDIRECTORY(src)

IF(BUILD_TESTS)
    ADD_SUBDIRECTORY(test)
ENDIF()

IF(BUILD_DOCS)
    ADD_SUBDIRECTORY(docsrc)
ENDIF()

IF(WITH_VIGRANUMPY)
    ADD_SUBDIRECTORY(vigranumpy)
ENDIF()

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/private/CMakeLists.txt)
    ADD_SUBDIRECTORY(private EXCLUDE_FROM_ALL)
ENDIF()


set(LIB_SUFFIX "" CACHE STRING "Define suffix of lib directory name (32/64)" )

##################################################
#
#     global installation commands
#
##################################################

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/vigra
        DESTINATION include)

##################################################
#
#     enable CPack package generation
#
##################################################
INCLUDE(CPackConfig)

##################################################
#
#     add target vigra-$version-src.tar.gz
#
##################################################

find_program(GIT_EXECUTABLE
              NAMES git git.exe git.cmd
              HINTS $ENV{ProgramFiles}/Git/bin
              DOC "git command line client")

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/package-src.cmake.in
                ${PROJECT_BINARY_DIR}/package-src.cmake
                @ONLY IMMEDIATE)

add_custom_target(PACKAGE_SRC_TAR
                   COMMAND ${CMAKE_COMMAND} -P package-src.cmake
                   WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                   COMMENT "Creating ${PROJECT_BINARY_DIR}/vigra-${vigra_version}-src.tar.gz")

ADD_DEPENDENCIES(PACKAGE_SRC_TAR check)
ADD_DEPENDENCIES(PACKAGE_SRC_TAR doc_cpp)
IF(WITH_VIGRANUMPY AND PYTHON_SPHINX)
    ADD_DEPENDENCIES(PACKAGE_SRC_TAR doc_python)
ENDIF()

##################################################
#
#  enable other non-CMake projects to find Vigra
#
##################################################

IF(NOT WIN32)
# vigra-config script for (non-CMake-based) third party packages
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/vigra-config.in
        ${PROJECT_BINARY_DIR}/bin/vigra-config
        @ONLY IMMEDIATE)
INSTALL(FILES ${PROJECT_BINARY_DIR}/bin/vigra-config
        DESTINATION bin
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
ENDIF(NOT WIN32)

##################################################
#
#     enable other CMake projects to find Vigra
#
##################################################

# config file for CMake FIND_PACKAGE command (for using the build tree)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/VigraConfigBuild.cmake.in
        ${PROJECT_BINARY_DIR}/VigraConfig.cmake
        @ONLY IMMEDIATE)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/VigraConfigVersion.cmake.in
        ${PROJECT_BINARY_DIR}/VigraConfigVersion.cmake
        @ONLY IMMEDIATE)

# config file for CMake FIND_PACKAGE command
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/VigraConfig.cmake.in
        ${PROJECT_BINARY_DIR}/lib/vigra/CMake/VigraConfig.cmake
        @ONLY IMMEDIATE)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/VigraConfigVersion.cmake.in
        ${PROJECT_BINARY_DIR}/lib/vigra/CMake/VigraConfigVersion.cmake
        @ONLY IMMEDIATE)

# export targets:
INSTALL(EXPORT vigra-targets
        DESTINATION lib${LIB_SUFFIX}/vigra)
INSTALL(FILES ${PROJECT_BINARY_DIR}/lib/vigra/CMake/VigraConfig.cmake
            ${PROJECT_BINARY_DIR}/lib/vigra/CMake/VigraConfigVersion.cmake
        DESTINATION lib${LIB_SUFFIX}/vigra)
EXPORT(TARGETS vigraimpex FILE vigra-targets.cmake)

##################################################
#
#     status output
#
##################################################

MESSAGE( STATUS "---------------------------------------------------------" )
MESSAGE( STATUS "VIGRA configuration information:" )
MESSAGE( STATUS "---------------------------------------------------------" )
MESSAGE( STATUS "  Using CXX standard: ${CMAKE_CXX_STANDARD}" )

IF(ZLIB_FOUND)
    MESSAGE( STATUS "  Using ZLIB  libraries: ${ZLIB_LIBRARIES}" )
ELSE()
    MESSAGE( STATUS "  ZLIB libraries not found (ZLIB support disabled)" )
ENDIF()

IF(PNG_FOUND)
    MESSAGE( STATUS "  Using PNG  libraries: ${PNG_LIBRARIES}" )
ELSE()
    MESSAGE( STATUS "  PNG libraries not found (PNG support disabled)" )
ENDIF()

IF(TIFF_FOUND)
    MESSAGE( STATUS "  Using TIFF libraries: ${TIFF_LIBRARIES}" )
ELSE()
    MESSAGE( STATUS "  TIFF libraries not found (TIFF support disabled)" )
ENDIF()

IF(JPEG_FOUND)
    MESSAGE( STATUS "  Using JPEG libraries: ${JPEG_LIBRARIES}" )
ELSE()
    MESSAGE( STATUS "  JPEG libraries not found (JPEG support disabled)" )
ENDIF()

IF(OPENEXR_FOUND)
    MESSAGE( STATUS "  Using OpenEXR  libraries: ${OPENEXR_LIBRARIES}" )
ELSEIF(NOT WITH_OPENEXR)
    MESSAGE( STATUS "  OpenEXR disabled by user (WITH_OPENEXR=0)" )
ELSE()
    MESSAGE( STATUS "  OpenEXR libraries not found (OpenEXR support disabled)" )
ENDIF()

IF(FFTW3_FOUND)
    MESSAGE( STATUS "  Using FFTW libraries: ${FFTW3_LIBRARIES}" )
ELSE()
    MESSAGE( STATUS "  FFTW libraries not found (FFTW support disabled)" )
ENDIF()

IF(HDF5_FOUND)
    MESSAGE( STATUS "  Using HDF5 libraries: ${HDF5_LIBRARIES}" )
ELSEIF(NOT WITH_HDF5)
    MESSAGE( STATUS "  HDF5 disabled by user (WITH_HDF5=0)" )
ELSE()
    MESSAGE( STATUS "  HDF5 libraries not found (HDF5 support disabled)" )
ENDIF()

IF(Boost_FOUND)
    MESSAGE( STATUS "  Using Boost Graph Library: ${Boost_INCLUDE_DIR}/boost/graph" )
ELSEIF(NOT WITH_BOOST_GRAPH)
    MESSAGE( STATUS "  Boost Graph Library disabled by user (WITH_BOOST_GRAPH=0)" )
ELSE()
    MESSAGE( STATUS "  Boost Graph Library not found (support disabled)" )
ENDIF()

IF(LEMON_FOUND)
    MESSAGE( STATUS "  Using LEMON graph library: ${LEMON_LIBRARY}" )
ELSEIF(NOT WITH_LEMON)
    MESSAGE( STATUS "  LEMON graph library disabled by user (WITH_LEMON=0)" )
ELSE()
    MESSAGE( STATUS "  LEMON graph library not found (support disabled)" )
ENDIF()

IF(WITH_VIGRANUMPY)
    IF(VIGRANUMPY_DEPENDENCIES_FOUND)
        MESSAGE( STATUS "  Using Python libraries: ${VIGRANUMPY_LIBRARIES}" )
        MESSAGE( STATUS "  Using Numpy includes:   ${Python_NumPy_INCLUDE_DIRS}" )
    ELSE()
        MESSAGE( STATUS "  Vigranumpy dependencies not found (vigranumpy disabled)" )
    ENDIF()
ELSE()
  MESSAGE( STATUS "  Vigranumpy disabled by user (WITH_VIGRANUMPY=0)" )
ENDIF()

MESSAGE( STATUS "---------------------------------------------------------" )

IF(VIGRA_STATIC_LIB)
    MESSAGE( STATUS "  building static lib" )
ELSE()
    MESSAGE( STATUS "  building shared lib" )
ENDIF()

MESSAGE( STATUS "  binaries will be generated in: ${CMAKE_CURRENT_BINARY_DIR}")
IF(DOXYGEN_FOUND AND Python_Interpreter_FOUND)
    MESSAGE( STATUS "  manuals will be generated in: ${DOCDIR}")
ENDIF()

MESSAGE( STATUS "---------------------------------------------------------" )

MESSAGE( STATUS "  includes will be installed at: ${CMAKE_INSTALL_PREFIX}/include")
MESSAGE( STATUS "  libraries will be installed at: ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

IF (MSVC)
  MESSAGE( STATUS "  binaries and DLLs will be installed at: ${CMAKE_INSTALL_PREFIX}/bin")
else()
  MESSAGE( STATUS "  binaries will be installed at: ${CMAKE_INSTALL_PREFIX}/bin")
endif()

IF(DOXYGEN_FOUND AND Python_Interpreter_FOUND)
  MESSAGE( STATUS "  vigra manuals will be installed at: "
    "${CMAKE_INSTALL_PREFIX}/${DOCINSTALL}/vigra/index.html")
ELSE()
  MESSAGE( STATUS "  vigra manuals cannot be generated (Doxygen or Python not found)")
ENDIF()

IF(WITH_VIGRANUMPY AND VIGRANUMPY_DEPENDENCIES_FOUND)
  MESSAGE( STATUS "  vigranumpy will be installed at ${CMAKE_INSTALL_PREFIX}/${VIGRANUMPY_INSTALL_DIR}" )
  IF(PYTHON_SPHINX)
    MESSAGE( STATUS "  vigranumpy manuals will be installed at: "
      "${CMAKE_INSTALL_PREFIX}/${DOCINSTALL}/vigranumpy/html/index.html")
  ELSE(PYTHON_SPHINX)
    MESSAGE( STATUS "  vigranumpy manuals cannot be generated "
      "(Sphinx missing)")
  ENDIF(PYTHON_SPHINX)
ELSE()
  MESSAGE( STATUS "  vigranumpy will NOT be installed" )
ENDIF()

IF(NOT TIFF_FOUND OR NOT JPEG_FOUND OR NOT PNG_FOUND OR NOT FFTW3_FOUND OR
   NOT HDF5_FOUND OR NOT VIGRANUMPY_DEPENDENCIES_FOUND)
  MESSAGE( STATUS "" )
  MESSAGE( STATUS "Consider setting DEPENDENCY_SEARCH_PREFIX to find missing libraries" )
ENDIF()

MESSAGE( STATUS "---------------------------------------------------------" )
