## Copyright 2020 Jefferson Amstutz
## SPDX-License-Identifier: Unlicense

function(print)
  foreach(arg ${ARGN})
    message("${arg} = ${${arg}}")
  endforeach()
endfunction()

macro(append_cmake_prefix_path)
  list(APPEND CMAKE_PREFIX_PATH ${ARGN})
  string(REPLACE ";" "|" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
endmacro()

macro(setup_subproject_path_vars _NAME)
  set(SUBPROJECT_NAME ${_NAME})

  set(SUBPROJECT_INSTALL_PATH ${INSTALL_DIR_ABSOLUTE})

  set(SUBPROJECT_SOURCE_PATH ${SUBPROJECT_NAME}/source)
  set(SUBPROJECT_STAMP_PATH ${SUBPROJECT_NAME}/stamp)
  set(SUBPROJECT_BUILD_PATH ${SUBPROJECT_NAME}/build)
  
endmacro()

macro(build_git_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL )
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    GIT_REPOSITORY  ${BUILD_SUBPROJECT_URL}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
     
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()

macro(build_minvr_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL PATCH)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  message("BUILD_SUBPROJECT_PATCH ${BUILD_SUBPROJECT_PATCH}")
  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    PATCH_COMMAND  git apply ${BUILD_SUBPROJECT_PATCH}/minvr_patch_09142021.patch
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    GIT_REPOSITORY  ${BUILD_SUBPROJECT_URL}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
      
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()



macro(build_CPPFSD_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    GIT_REPOSITORY  ${BUILD_SUBPROJECT_URL}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
     
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -DBUILD_SHARED_LIBS=OFF
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()



macro(build_svn_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    SVN_REPOSITORY   ${BUILD_SUBPROJECT_URL}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
     
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()

macro(build_glew_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

   message(STATUS "SUBPROJECT_INSTALL_PATH: ${SUBPROJECT_INSTALL_PATH}")

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    URL   ${BUILD_SUBPROJECT_URL}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
	  -S ${CMAKE_CURRENT_SOURCE_DIR}/glew/source/build/cmake
    
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()
macro(build_glfw_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    GIT_REPOSITORY  ${BUILD_SUBPROJECT_URL}
	GIT_TAG gpu-affinity
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS

      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -DGLFW_BUILD_EXAMPLES=OFF
	  -DGLFW_BUILD_TESTS=OFF
	  -DGLFW_BUILD_DOCS=OFF
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()


macro(build_glm_subproject)
  # See cmake_parse_arguments docs to see how args get parsed here:
  #    https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
  set(oneValueArgs NAME URL TAG)
  set(multiValueArgs BUILD_ARGS DEPENDS_ON)
  cmake_parse_arguments(BUILD_SUBPROJECT "" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Setup SUBPROJECT_* variables (containing paths) for this function
  setup_subproject_path_vars(${BUILD_SUBPROJECT_NAME})

  # Build the actual subproject
  ExternalProject_Add(${SUBPROJECT_NAME}
    PREFIX ${SUBPROJECT_NAME}
    DOWNLOAD_DIR ${SUBPROJECT_NAME}
    STAMP_DIR ${SUBPROJECT_STAMP_PATH}
    SOURCE_DIR ${SUBPROJECT_SOURCE_PATH}
    BINARY_DIR ${SUBPROJECT_BUILD_PATH}
    GIT_REPOSITORY  ${BUILD_SUBPROJECT_URL}
	  GIT_TAG ${BUILD_SUBPROJECT_TAG}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS

      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${SUBPROJECT_INSTALL_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -DGLFW_BUILD_EXAMPLES=OFF
      -DBUILD_TESTING=OFF
      -DGLM_TEST_ENABLE=OFF
	  
      ${BUILD_SUBPROJECT_BUILD_ARGS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  if(BUILD_SUBPROJECT_DEPENDS_ON)
    ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME}
      configure ${BUILD_SUBPROJECT_DEPENDS_ON}
    )
  endif()

  # Place installed component on CMAKE_PREFIX_PATH for downstream consumption
  append_cmake_prefix_path(${SUBPROJECT_INSTALL_PATH})
endmacro()