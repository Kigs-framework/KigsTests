project(projects)

if(NOT "${KIGS_PLATFORM}" STREQUAL "Android")
	add_subdirectory(Arbres)
	add_subdirectory(MeshSimplifier)
	add_subdirectory(IcosahedronNSpacePartition)
endif()