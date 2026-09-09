usd_plugin(usdImagingSmiley

    CPPFILES
        smileyAdapter.cpp
        smileyMeshGenerator.cpp
        smileyPrimDataSource.cpp
        smileyProcedural.cpp
        tokens.cpp

    LIBRARIES
        arch
        js
        plug
        usd
        tf
        sdf
        vt
        gf
        hd
        hdGp
        usdGeom
        usdImaging
        usdSmiley

    RESOURCE_FILES
        plugInfo.json
)