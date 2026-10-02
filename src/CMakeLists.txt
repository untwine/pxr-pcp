configure_file(
    pxr/pcp/pxr.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/pxr/pcp/pxr.h
    @ONLY
)

add_library(pcp
    pxr/pcp/arc.cpp
    pxr/pcp/cache.cpp
    pxr/pcp/changes.cpp
    pxr/pcp/composeSite.cpp
    pxr/pcp/debugCodes.cpp
    pxr/pcp/dependencies.cpp
    pxr/pcp/dependency.cpp
    pxr/pcp/dependentNamespaceEditUtils.cpp
    pxr/pcp/diagnostic.cpp
    pxr/pcp/dynamicFileFormatContext.cpp
    pxr/pcp/dynamicFileFormatDependencyData.cpp
    pxr/pcp/dynamicFileFormatInterface.cpp
    pxr/pcp/errors.cpp
    pxr/pcp/expressionVariables.cpp
    pxr/pcp/expressionVariablesDependencyData.cpp
    pxr/pcp/expressionVariablesSource.cpp
    pxr/pcp/instanceKey.cpp
    pxr/pcp/instancing.cpp
    pxr/pcp/iterator.cpp
    pxr/pcp/layerRelocatesEditBuilder.cpp
    pxr/pcp/layerStack.cpp
    pxr/pcp/layerStackIdentifier.cpp
    pxr/pcp/layerStackRegistry.cpp
    pxr/pcp/mapExpression.cpp
    pxr/pcp/mapFunction.cpp
    pxr/pcp/namespaceEdits.cpp
    pxr/pcp/node.cpp
    pxr/pcp/node_Iterator.cpp
    pxr/pcp/pathTranslation.cpp
    pxr/pcp/primIndex.cpp
    pxr/pcp/primIndex_Graph.cpp
    pxr/pcp/primIndex_StackFrame.cpp
    pxr/pcp/propertyIndex.cpp
    pxr/pcp/site.cpp
    pxr/pcp/statistics.cpp
    pxr/pcp/strengthOrdering.cpp
    pxr/pcp/targetIndex.cpp
    pxr/pcp/types.cpp
    pxr/pcp/utils.cpp
)

target_include_directories(pcp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compile features can only raise the effective standard, so this can't be
# undercut by a toolchain file that pre-sets CMAKE_CXX_STANDARD lower.
target_compile_features(pcp PUBLIC cxx_std_17)

target_link_libraries(pcp
    PUBLIC
        pxr::arch
        pxr::tf
        pxr::trace
        pxr::vt
        pxr::sdf
        pxr::work
        pxr::ar
        TBB::tbb
)

if(NOT WIN32 AND NOT SKBUILD)
    set_target_properties(pcp PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(BUILD_PYTHON_BINDINGS)
    target_sources(pcp
        PRIVATE
            pxr/pcp/moduleDeps.cpp
            pxr/pcp/pyUtils.cpp
    )

    target_link_libraries(pcp
        PUBLIC
            pxr::boost-python
    )

    add_subdirectory(python)
endif()

if (ENABLE_PRECOMPILED_HEADERS)
    target_precompile_headers(pcp
        PRIVATE
            pxr/pcp/pch.h
    )
endif()

if(MSVC)
    target_compile_options(pcp PRIVATE /bigobj)
endif()

set_target_properties(pcp
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "PxrPcp"
)

target_compile_definitions(pcp
    PRIVATE
        PCP_EXPORTS=1
        MFB_PACKAGE_MODULE=Pcp
        MFB_PACKAGE_NAME=pcp
        MFB_ALT_PACKAGE_NAME=pcp
)

target_sources(pcp
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/pxr/pcp/pxr.h
            pxr/pcp/api.h
            pxr/pcp/arc.h
            pxr/pcp/cache.h
            pxr/pcp/changes.h
            pxr/pcp/composeSite.h
            pxr/pcp/dependency.h
            pxr/pcp/dependentNamespaceEditUtils.h
            pxr/pcp/dynamicFileFormatContext.h
            pxr/pcp/dynamicFileFormatDependencyData.h
            pxr/pcp/dynamicFileFormatInterface.h
            pxr/pcp/errors.h
            pxr/pcp/expressionVariables.h
            pxr/pcp/expressionVariablesDependencyData.h
            pxr/pcp/expressionVariablesSource.h
            pxr/pcp/instanceKey.h
            pxr/pcp/iterator.h
            pxr/pcp/layerRelocatesEditBuilder.h
            pxr/pcp/layerStack.h
            pxr/pcp/layerStackIdentifier.h
            pxr/pcp/mapExpression.h
            pxr/pcp/mapFunction.h
            pxr/pcp/namespaceEdits.h
            pxr/pcp/node.h
            pxr/pcp/pathTranslation.h
            pxr/pcp/primIndex.h
            pxr/pcp/propertyIndex.h
            pxr/pcp/site.h
            pxr/pcp/strengthOrdering.h
            pxr/pcp/targetIndex.h
            pxr/pcp/types.h
)

if(BUILD_PYTHON_BINDINGS)
    target_sources(pcp
        PUBLIC
            FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}
            FILES
                pxr/pcp/pyUtils.h
    )
endif()

install(
    TARGETS pcp EXPORT ${PROJECT_NAME}
    FILE_SET public_headers
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT ${PROJECT_NAME}
    FILE pxr-pcp-targets.cmake
    NAMESPACE pxr::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/pxr-pcp
)
