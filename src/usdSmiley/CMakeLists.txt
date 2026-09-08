usd_library(usdSmiley

    PUBLIC_HEADERS_INSTALL_PREFIX
        ${ORGANIZATION}

    PYTHON_INSTALL_PREFIX
        ${ORGANIZATION}

    LIBRARIES
        js
        plug
        tf
        sdf
        vt
        gf
        usd
        usdGeom
        arch

    PUBLIC_CLASSES
        smiley

    PUBLIC_HEADERS
        api.h
        tokens.h

    CPPFILES
        tokens.cpp

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapTokens.cpp
        wrapSmiley.cpp

    PYTHON_FILES
        __init__.py

    RESOURCE_FILES
        generatedSchema.usda
        plugInfo.json
)
