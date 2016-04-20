get_filename_component(myDir ${CMAKE_CURRENT_LIST_FILE} PATH) # get the directory where I myself am
get_filename_component(rootDir ${myDir}/../../../ ABSOLUTE) # get the chosen install prefix

# set the version of myself
set(EVPATH_VERSION_MAJOR 4)
set(EVPATH_VERSION_MINOR 0)
set(EVPATH_VERSION_PATCH 128)
set(EVPATH_VERSION ${EVPATH_VERSION_MAJOR}.${EVPATH_VERSION_MINOR}.${EVPATH_VERSION_PATCH} )

# what is my include directory
set(EVPATH_INCLUDES "${rootDir}/include")

# import the exported targets
include(${myDir}/EVPathTargets.cmake)

# set the expected library variable
set(EVPATH_LIBRARIES evpath atl ffs cercs_env )

