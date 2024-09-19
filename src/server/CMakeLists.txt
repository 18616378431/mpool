add_subdirectory(apps)

if ((APPS_BUILD AND NOT APPS_BUILD STREQUAL "none"))
  add_subdirectory(database)
endif()
