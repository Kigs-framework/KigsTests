project(MeshSimplifier)

if(${KIGS_PLATFORM} STREQUAL "Android" OR ${KIGS_PLATFORM} STREQUAL "WUP")
	add_library(MeshSimplifier SHARED "")
else()
	add_executable(MeshSimplifier "")
endif()

if(${KIGS_PLATFORM} STREQUAL "Windows")
	set(CMAKE_SHARED_LINKER_FLAGS_STATICDEBUG "${CMAKE_SHARED_LINKER_FLAGS_STATICDEBUG} /SAFESEH:NO /DEBUG:FULL")
	set(CMAKE_SHARED_LINKER_FLAGS_STATICRELEASE "${CMAKE_SHARED_LINKER_FLAGS_STATICRELEASE} /SUBSYSTEM:WINDOWS /SAFESEH:NO")
	set(CMAKE_SHARED_LINKER_FLAGS_STATICRELEASETOOLS "${CMAKE_SHARED_LINKER_FLAGS_STATICRELEASETOOLS} /SUBSYSTEM:WINDOWS /DEBUG:FULL /SAFESEH:NO")
endif()

set(MeshSimplifier_GUID_CMAKE "896F4B63-E870-46C8-BE8A-6B4A6C20EEE7" CACHE INTERNAL "MeshSimplifier GUID")

target_link_libraries(MeshSimplifier PRIVATE DataDrivenApplication OMeshSimplification)
#target_link_libraries(MeshSimplifier PRIVATE Holographic)

target_include_directories(MeshSimplifier PRIVATE "Headers")


file(GLOB_RECURSE all_sources "Sources/*.cpp")
file(GLOB_RECURSE all_headers "Headers/*.h")

target_sources(MeshSimplifier 
	PRIVATE 
		${all_sources}
		${all_headers}
		)
set_property(TARGET MeshSimplifier PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Android/assets")

#add custom prebuild asset generation
if(NOT ${KIGS_PLATFORM} STREQUAL "Javascript")
	add_custom_command(TARGET MeshSimplifier  PRE_BUILD 
											 COMMAND AssetManager.exe -i ${CMAKE_CURRENT_SOURCE_DIR}/Ressources -t ${CMAKE_CURRENT_SOURCE_DIR}/TmpRessources -o ${CMAKE_CURRENT_SOURCE_DIR}/Android/assets
											 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../kigs/tools/win32_executable/)
endif()

if(${KIGS_PLATFORM} STREQUAL "WUP")
	target_link_libraries(MeshSimplifier PRIVATE zlib)
	include_external_msproject(MeshSimplifier_wup_packaging "${CMAKE_CURRENT_SOURCE_DIR}/Build/UWP/MeshSimplifierPackaging.vcxproj")
elseif(${KIGS_PLATFORM} STREQUAL "Android")
	include_external_msproject(MeshSimplifier_android_packaging "${CMAKE_CURRENT_SOURCE_DIR}/Android/kigs.MeshSimplifier.Packaging.androidproj" TYPE "39E2626F-3545-4960-A6E8-258AD8476CE5")
	include_external_msproject(MeshSimplifier_kigs_java "${KIGS_FRAMEWORK}/PlatformsModules/Android/Platform/Main/kigsmain/kigs.kigsmain.androidproj" TYPE "39E2626F-3545-4960-A6E8-258AD8476CE5")
elseif(${KIGS_PLATFORM} STREQUAL "Javascript")
	set(CMAKE_EXECUTABLE_SUFFIX ".js")
	set_target_properties(MeshSimplifier PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Android/assets@/  --js-library ${KIGS_PLATFORM_ROOT}/Platform/2DLayers/2DLayers_JavaScript.js --js-library ${KIGS_PLATFORM_ROOT}/Platform/GUI/GUI_JavaScript.js --js-library ${KIGS_PLATFORM_ROOT}/Platform/Input/Input_JavaScript.js --js-library ${KIGS_PLATFORM_ROOT}/Platform/Renderer/Renderer_JavaScript.js --js-library ${KIGS_PLATFORM_ROOT}/Platform/Sound/Audio.js -s TOTAL_MEMORY=67108864 -s EXPORTED_FUNCTIONS='[_main]' --use-preload-plugins -s WASM=1 -s BINARYEN_METHOD='native-wasm' -s ALLOW_MEMORY_GROWTH=1" )
endif()
