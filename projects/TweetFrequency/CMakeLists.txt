project(TweetFrequency)

if(${KIGS_PLATFORM} STREQUAL "Windows" )
	add_executable(TweetFrequency "")
	

	set(CMAKE_SHARED_LINKER_FLAGS_STATICDEBUG "${CMAKE_SHARED_LINKER_FLAGS_STATICDEBUG} /SAFESEH:NO /DEBUG:FULL")
	set(CMAKE_SHARED_LINKER_FLAGS_STATICRELEASE "${CMAKE_SHARED_LINKER_FLAGS_STATICRELEASE} /SUBSYSTEM:WINDOWS /SAFESEH:NO")
	set(CMAKE_SHARED_LINKER_FLAGS_STATICRELEASETOOLS "${CMAKE_SHARED_LINKER_FLAGS_STATICRELEASETOOLS} /SUBSYSTEM:WINDOWS /DEBUG:FULL /SAFESEH:NO")


	set(TweetFrequency_GUID_CMAKE "3CF2CB52-7E97-4ECD-A459-8940F3B00B43" CACHE INTERNAL "TweetFrequency GUID")

	target_link_libraries(TweetFrequency PRIVATE DataDrivenApplication HTTPRequest)

	target_include_directories(TweetFrequency PRIVATE "Headers")


	file(GLOB_RECURSE all_sources "Sources/*.cpp")
	file(GLOB_RECURSE all_headers "Headers/*.h")

	target_sources(TweetFrequency 
		PRIVATE 
			${all_sources}
			${all_headers}
			)
	set_property(TARGET TweetFrequency PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Data")

endif()
