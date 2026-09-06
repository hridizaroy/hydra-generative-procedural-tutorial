set(PXR_PREFIX pxr/usdImaging)
set(PXR_PACKAGE usdImagingSmiley)

pxr_library(usdImagingSmiley
    LIBRARIES
        gf
        hd
        hdGp
        tf
        usd
        usdGeom
        usdSmiley
        usdImaging

    PUBLIC_CLASSES
        smileyAdapter
        tokens

    PRIVATE_CLASSES
        smileyProcedural

    PUBLIC_HEADERS
        api.h

    RESOURCE_FILES
        plugInfo.json
)
