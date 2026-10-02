add_library(TestPcpStreamingLayerReload SHARED plugin.cpp)
target_link_libraries(TestPcpStreamingLayerReload PUBLIC pcp pxr::sdf pxr::tf)
file(
    GENERATE
    OUTPUT "$<TARGET_FILE_DIR:TestPcpStreamingLayerReload>/plugInfo_$<CONFIG>.json"
    INPUT plugInfo.json
)
