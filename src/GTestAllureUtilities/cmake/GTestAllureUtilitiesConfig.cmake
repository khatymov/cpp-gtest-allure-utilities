@PACKAGE_INIT@

# If you ever add external dependencies that should be auto-found for users,
# you can add find_dependency() calls here.

include("${CMAKE_CURRENT_LIST_DIR}/GTestAllureUtilitiesTargets.cmake")

check_required_components(GTestAllureUtilities)
