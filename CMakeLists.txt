cmake_minimum_required(VERSION 3.24)

project(
	CraftingCategories
	VERSION 1.0.0
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
)
include(SKSEPlugin)

set(SKSE_USE_XBYAK OFF)
set(SKSE_SUPPORT_VR OFF)

SKSEPlugin_Add(
	${PROJECT_NAME}
	SOURCE_DIR src
	SOURCES
		.clang-format
		.editorconfig
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/PCH/PCH.h
)

find_package(jsoncpp CONFIG REQUIRED)
find_package(tsl-ordered-map CONFIG REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		jsoncpp_static
		tsl::ordered_map
)

file(GLOB TRANSLATIONS
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/data/Interface/Translations/CraftingCategories_*.txt"
)

file(GLOB CONFIGS
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/data/SKSE/Plugins/CraftingCategories/*.json"
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		${TRANSLATIONS}
		${CONFIGS}
)

source_group(
	"Data Files/Translations"
	FILES
		${TRANSLATIONS}
)

source_group(
	"Data Files/Configs"
	FILES
		${CONFIGS}
)

install(
	FILES
		"${PROJECT_SOURCE_DIR}/data/Interface/constructibleobjectmenu.swf"
	DESTINATION
		"Interface"
)

install(
	FILES
		${TRANSLATIONS}
	DESTINATION
		"Interface/Translations"
)

install(
	FILES
		"${PROJECT_SOURCE_DIR}/data/Interface/craftingcategories/icons.swf"
	DESTINATION
		"Interface/craftingcategories"
)

install(
	FILES
		${CONFIGS}
	DESTINATION
		"SKSE/Plugins/CraftingCategories"
)

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)
