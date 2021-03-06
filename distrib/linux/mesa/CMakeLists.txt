project(Mesa)

# Name and version of our package

set(PACKAGE_NAME Mesa)
set(PACKAGE_VERSION 17.1.0)

# Version of our library

set(MAJOR_LIBRARY_VERSION 1)
set(LIBRARY_VERSION ${MAJOR_LIBRARY_VERSION}.5)

# Git tag for our library

set(GIT_TAG v17.1.0)

# Specify where our local package will be installed

set(FULL_LOCAL_EXTERNAL_PACKAGE_DIR ${PROJECT_SOURCE_DIR}/${LOCAL_EXTERNAL_PACKAGE_DIR})

# Name of our package's shared library

set(SHARED_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}GL${CMAKE_SHARED_LIBRARY_SUFFIX}.${MAJOR_LIBRARY_VERSION})

# Specify the files that need to have their SHA-1 value checked

set(SHA1_FILES
    lib/${SHARED_LIBRARY}
)

# Use the pre-built version of our package unless instructed otherwise

if(USE_PREBUILT_MESA_PACKAGE)
    # Retrieve the library's package

    string(REPLACE "${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/" ""
           RELATIVE_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${REMOTE_EXTERNAL_PACKAGE_DIR}")

    retrieve_package_file(${PACKAGE_NAME} ${PACKAGE_VERSION}
                          ${RELATIVE_PROJECT_SOURCE_DIR} 2a677c7ecd522a6682a12ee64234a8b78d17686c
                          SHA1_FILES ${SHA1_FILES}
                          SHA1_VALUES 8988c322d8055b34f2a56dd0a8ebd327883fb3a5)
else()
    # Build Mesa as an external project

    set(PACKAGE_BUILD ${PACKAGE_NAME}Build)
    set(PACKAGE_BUILD_DIR ${CMAKE_SOURCE_DIR}/ext/${PACKAGE_NAME})

    ExternalProject_Add(${PACKAGE_BUILD}
        DOWNLOAD_DIR
            ${PACKAGE_BUILD_DIR}
        SOURCE_DIR
            ${PACKAGE_BUILD_DIR}
        BINARY_DIR
            ${PACKAGE_BUILD_DIR}
        INSTALL_DIR
            ${FULL_LOCAL_EXTERNAL_PACKAGE_DIR}
        GIT_REPOSITORY
            https://github.com/opencor/mesa
        GIT_TAG
            ${GIT_TAG}
        CONFIGURE_COMMAND
            ""
        BUILD_COMMAND
            scons build=release libgl-xlib
        INSTALL_COMMAND
            cp ${PACKAGE_BUILD_DIR}/build/linux-x86_64/gallium/targets/libgl-xlib/${SHARED_LIBRARY} <INSTALL_DIR>/lib
        BUILD_BYPRODUCTS
            <INSTALL_DIR>/lib/${SHARED_LIBRARY}
    )

    # Package our external project

    create_package_file(${PACKAGE_NAME} ${PACKAGE_VERSION}
                        ${LOCAL_EXTERNAL_PACKAGE_DIR}
                        PACKAGED_FILES ${SHA1_FILES}
                        SHA1_FILES ${SHA1_FILES}
                        TARGET ${PACKAGE_BUILD})
endif()

# Allow our main CMake project to find our Mesa package

set(MESA_ROOT_DIR ${FULL_LOCAL_EXTERNAL_PACKAGE_DIR} PARENT_SCOPE)
set(MESA_LIBRARY ${FULL_LOCAL_EXTERNAL_PACKAGE_DIR}/lib/${SHARED_LIBRARY} PARENT_SCOPE)

# Copy our copy of the Mesa library

if(USE_PREBUILT_MESA_PACKAGE)
    set(COPY_TARGET DIRECT)
else()
    set(COPY_TARGET ${PACKAGE_BUILD})
endif()

copy_file_to_build_dir(${COPY_TARGET} ${FULL_LOCAL_EXTERNAL_PACKAGE_DIR}/lib lib ${SHARED_LIBRARY})
