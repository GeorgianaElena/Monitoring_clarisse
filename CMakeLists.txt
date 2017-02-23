#http://www.cmake.org/examples/
cmake_minimum_required (VERSION 2.8.3)
cmake_policy(VERSION 2.8.3)


# the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1" 
MESSAGE( STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )
# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
MESSAGE( STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME} )
# only the version part of CMAKE_SYSTEM 
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION} )
# the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )


find_path(BG_SYSTEM bgsys PATHS /)
if(BG_SYSTEM)
  message(STATUS "Blue Gene system")
endif()

find_path(CRAY_SYSTEM work PATHS /)
if(CRAY_SYSTEM)
  message(STATUS "Cray system")
endif()

#-----------------------------------------------------------#
# Force MPI compilers
#-----------------------------------------------------------#
include(CMakeForceCompiler)

if(CRAY_SYSTEM)
	CMAKE_FORCE_C_COMPILER(cc "MPI C Compiler")
else()
	CMAKE_FORCE_C_COMPILER(mpicc "MPI C Compiler")
endif()

#project def 
project (monitoring)
# The version number.
set (MONITORING_VERSION_MAJOR 1)
set (MONITORING_VERSION_MINOR 0)


#necessary for off64_t
add_definitions(-D_LARGEFILE64_SOURCE)

#set compile flags
#option(CLARISSE_TIMING "Use CLARISSE_TIMING" OFF)

#if (CLARISSE_TIMING)
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCLARISSE_TIMING")
#endif ()
 
include_directories (include)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=gnu11") 
#add library
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

set (SRCS metrics_aggregator.c evp_monitoring.c helpers.c metrics_crawler.c metric_type.c storage.c)
set (HDRS metrics_aggregator.h evp_monitoring.h helpers.h metrics_crawler.h metric_type.h storage.h uthash.h)
set (PROGS test)

add_library(evp_monitoring ${SRCS})
find_package (Threads REQUIRED)
set (EXTRA_LIBS ${EXTRA_LIBS} evp_monitoring ${CMAKE_THREAD_LIBS_INIT} papi) 


set(COMMON_EVPATH_INCLUDE_PATH "../evpath/include")
set(COMMON_EVPATH_LIBRARY_PATH "../evpath/lib")

set(EVPATH_INCLUDE_PATH ${COMMON_EVPATH_INCLUDE_PATH} REQUIRED CACHE PATH "docstring")
set(EVPATH_LIBRARY_PATH ${COMMON_EVPATH_LIBRARY_PATH} REQUIRED CACHE PATH "docstring")
include_directories (${EVPATH_INCLUDE_PATH})
find_library(EVPATH_LIB evpath ${EVPATH_LIBRARY_PATH})
set (EXTRA_LIBS ${EXTRA_LIBS} ${EVPATH_LIB})
unset(EVPATH_LIB CACHE)

#MESSAGE("CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH} )
#MESSAGE("EXTRA_LIBS: " ${EXTRA_LIBS} )

set(FFS_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
set(FFS_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
include_directories (${FFS_INCLUDE_PATH})
find_library(FFS_LIB ffs ${FFS_LIBRARY_PATH})
set (EXTRA_LIBS ${EXTRA_LIBS} ${FFS_LIB})
unset(FFS_LIB CACHE)

#MESSAGE("CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH} )
#MESSAGE("EXTRA_LIBS: " ${EXTRA_LIBS} )

set(DILL_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
set(DILL_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
include_directories (${DILL_INCLUDE_PATH})
find_library(DILL_LIB dill ${DILL_LIBRARY_PATH})
set (EXTRA_LIBS ${EXTRA_LIBS} ${DILL_LIB})
unset(DILL_LIB CACHE)

set(ATL_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
set(ATL_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
include_directories (${ATL_INCLUDE_PATH})
find_library(ATL_LIB atl ${ATL_LIBRARY_PATH})
set (EXTRA_LIBS ${EXTRA_LIBS} ${ATL_LIB})
unset(ATL_LIB CACHE)

set(CERCS_ENV_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
set(CERCS_ENV_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
include_directories (${CERCS_ENV_INCLUDE_PATH})
find_library(CERCS_ENV_LIB cercs_env ${CERCS_ENV_LIBRARY_PATH})
set (EXTRA_LIBS ${EXTRA_LIBS} ${CERCS_ENV_LIB})
unset(CERCS_ENV_LIB CACHE)

if(CRAY_SYSTEM)
  set(ENET_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
  set(ENET_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
  include_directories (${ENET_INCLUDE_PATH})
  find_library(ENET_LIB enet ${ENET_LIBRARY_PATH})
  set (EXTRA_LIBS ${EXTRA_LIBS} ${ENET_LIB})
  unset(ENET_LIB CACHE)

  set(TRIOS_NNTI_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
  set(TRIOS_NNTI_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
  include_directories (${TRIOS_NNTI_INCLUDE_PATH})
  find_library(TRIOS_NNTI_LIB trios_nnti ${TRIOS_NNTI_LIBRARY_PATH})
  set (EXTRA_LIBS ${EXTRA_LIBS} ${TRIOS_NNTI_LIB})
  unset(TRIOS_NNTI_LIB CACHE)

  set(TRIOS_SUPPORT_INCLUDE_PATH "${COMMON_EVPATH_INCLUDE_PATH}" REQUIRED CACHE PATH "docstring")
  set(TRIOS_SUPPORT_LIBRARY_PATH "${COMMON_EVPATH_LIBRARY_PATH}" REQUIRED CACHE PATH "docstring")
  include_directories (${TRIOS_SUPPORT_INCLUDE_PATH})
  find_library(TRIOS_SUPPORT_LIB trios_support ${TRIOS_SUPPORT_LIBRARY_PATH})
  set (EXTRA_LIBS ${EXTRA_LIBS} ${TRIOS_SUPPORT_LIB})
  unset(TRIOS_SUPPORT_LIB CACHE)


endif()


unset(COMMON_EVPATH_LIBRARY_PATH CACHE)
unset(COMMON_EVPATH_INCLUDE_PATH CACHE)

foreach (PROG ${PROGS})
    add_executable(${PROG} ${PROG}.c)
    target_link_libraries(${PROG} LINK_PUBLIC ${EXTRA_LIBS})
endforeach()