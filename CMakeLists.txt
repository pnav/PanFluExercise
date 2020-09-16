project(Exercise)

cmake_minimum_required(VERSION 2.4.0)

# keep the library path in installed executables
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# path for additional modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
    set(CMAKE_CXX_FLAGS "/Ox /W0 /EHsc")
    add_definitions(-DWIN32)
else(WIN32)
    # show all warnings
    set(CMAKE_CXX_FLAGS "-Wall")
endif(WIN32)

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GDAL_LIBRARY})

# NetCDF support optional
set(USE_NETCDF OFF CACHE BOOL "NetCDF file support.")

if(USE_NETCDF)
    add_definitions(-DUSE_NETCDF)
    find_package(NetCDF REQUIRED)
    include_directories(${NetCDF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${NetCDF_LIBRARIES})
endif(USE_NETCDF)

find_package(Blitz REQUIRED)
include_directories(${Blitz_INCLUDE_DIRS})
set(LIBS ${LIBS} ${BLITZ_LIBRARIES})

# find_package(VTK REQUIRED)
# include(${VTK_USE_FILE})
# set(LIBS ${LIBS} vtkGUISupportQt vtkChartsCore vtkRenderingCore vtkRenderingFreeTypeOpenGL vtkRenderingVolumeOpenGL vtkViewsContext2D vtkIOExport)

find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSL_LIBRARIES})

set(SRCS ${SRCS}
    src/EpidemicDataSet.cpp
    src/EpidemicSimulation.cpp
    src/log.cpp
    src/main.cpp
    src/Npi.cpp
    src/Parameters.cpp
    src/PriorityGroup.cpp
    src/PriorityGroupSelections.cpp
    src/Stockpile.cpp
    src/StockpileNetwork.cpp
    src/StockpileNetworkDistribution.cpp
    src/models/random.cpp
    src/models/disease/iliView.cpp
    src/models/disease/StochasticSEATIRD.cpp
    src/models/disease/StochasticSEATIRDSchedule.cpp
)


add_executable(exercise MACOSX_BUNDLE WIN32
    ${SRCS})

target_link_libraries(exercise ${LIBS})

# install executable
INSTALL(TARGETS exercise
    RUNTIME DESTINATION bin COMPONENT Runtime
    BUNDLE DESTINATION . COMPONENT Runtime
)


# on Apple platform make the .app bundle; on Windows make the installer
if(APPLE OR WIN32)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Exercise.")
    set(CPACK_PACKAGE_EXECUTABLES "exercise" "Exercise")
    set(CPACK_PACKAGE_NAME "Exercise")
    set(CPACK_PACKAGE_VENDOR "The University of Texas at Austin")
    set(CPACK_PACKAGE_VERSION_MAJOR 1)
    set(CPACK_PACKAGE_VERSION_MINOR 1)
    set(CPACK_PACKAGE_VERSION_PATCH 0)

    if(APPLE)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/Exercise.app")
        SET(DATA_DEST_DIR Exercise.app/Contents/Resources)
    elseif(WIN32)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Exercise.exe")
        SET(DATA_DEST_DIR .)
    endif()

    # install data files
    INSTALL(DIRECTORY data DESTINATION ${DATA_DEST_DIR} COMPONENT Runtime)

    if(APPLE)
        # directories to look for dependencies
        # set(DEPENDENCY_DIRS ${VTK_LIBRARY_DIRS})
    elseif(WIN32)
        # directories to look for dependencies
        #AARONBAD -- need to manually specify VTK binaries dir where dll's live. Everything else is ok.
        # set(DEPENDENCY_DIRS ${QT_BINARY_DIR} "C:/Users/amk3264/Downloads/gdalwin32-1.4.1/bin/" "C:/Users/amk3264/Downloads/VTK-6.0.0-win32-install/bin")
        include(InstallRequiredSystemLibraries)
    endif()

    # to create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" for example
    # where CPackConfig.cmake is created by including CPack
    if(APPLE)
        set(CPACK_BINARY_DRAGNDROP ON)
    elseif(WIN32)
        set(CPACK_BINARY_NSIS ON)
    endif()

    include(CPack)
else()
    # Linux

    # install data files
    INSTALL(DIRECTORY data DESTINATION . COMPONENT Runtime)

    # install utility files (Linux only)
    INSTALL(DIRECTORY util DESTINATION . COMPONENT Runtime USE_SOURCE_PERMISSIONS)
endif()
