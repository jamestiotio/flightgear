

# placehodler target for other ones to depend upon
add_custom_target(
    debug_symbols
)

function(export_debug_symbols target)

    if (APPLE)
        add_custom_target(${target}.dSYM
            COMMENT "Generating dSYM files for ${target}"
            COMMAND dsymutil --out=${CMAKE_BINARY_DIR}/symbols ${target}.dSYM $<TARGET_FILE:${target}>
            DEPENDS $<TARGET_FILE:${target}>
        ) 

        install(DIRECTORY ${CMAKE_BINARY_DIR}/symbols/${target}.dSYM DESTINATION symbols OPTIONAL)

        add_dependencies(debug_symbols ${target}.dSYM)
    endif()

    if (MSVC)
        # TODO: ensure PDBs end up in ${CMAKE_BINARY_DIR}/symbols
    endif()
endfunction()


add_custom_target(upload_debug_symbols
    COMMENT "Uploading debug symbols via sentry-cli"
    COMMAND export SENTRY_ORG=flightgear
    COMMAND export SENTRY_PROJECT=flightgear
    COMMAND sentry-cli upload-dif ${CMAKE_BINARY_DIR}/symbols
) 

add_dependencies(upload_debug_symbols debug_symbols)

