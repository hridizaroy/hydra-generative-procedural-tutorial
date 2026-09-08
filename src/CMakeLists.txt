# A top-level name used as the root directory for public headers
# installation, and the root python module.
# This is optional.  See ./usdTri/CMakeLists.txt for how it's used.
set(ORGANIZATION "usdsmileygenproc")

# Build & install libraries.
add_subdirectory(usdSmiley)
add_subdirectory(usdImagingSmiley)

# Install top-level plugInfo for including per-library plugInfo(s).
install(
    FILES plugInfo.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${USD_PLUG_INFO_ROOT_DIR}
)

# Install top-level plugInfo for including per-plugin plugInfo(s).
install(
    FILES plugInfo.json
    DESTINATION ${USD_PLUGIN_DIR}/${USD_PLUG_INFO_ROOT_DIR}
)

# Install top level python __init__.py
install(
    FILES __init__.py
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${USD_PYTHON_DIR}/${ORGANIZATION}
)
