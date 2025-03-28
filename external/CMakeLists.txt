# mGE
add_subdirectory(mge)

# nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(json)

# nativefiledialog-external
add_subdirectory(nativefiledialog-extended)