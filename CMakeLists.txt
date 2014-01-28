################################################################################
# preparation
#
cmake_minimum_required (VERSION 2.8)
project (ProtocolLibrary)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo )
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations to those we need" FORCE)

### Variables describing project layout
set(${PROJECT_NAME}_GENERATED_FILES_DIR "${${PROJECT_NAME}_BINARY_DIR}/midl")
set(${PROJECT_NAME}_RC_DIR ${${PROJECT_NAME}_SOURCE_DIR}/res)

### where are the files?
set(${PROJECT_NAME}_SRC_DIR_PROT_HANDLER "${${PROJECT_NAME}_SOURCE_DIR}/handler")
set(${PROJECT_NAME}_SRC_DIR_HTTP_PARSER "${${PROJECT_NAME}_SOURCE_DIR}/http-parser-master")
set(${PROJECT_NAME}_SRC_DIR_PAPP "${${PROJECT_NAME}_SOURCE_DIR}/papp")
set(${PROJECT_NAME}_SRC_DIR_PROT_PATCHING "${${PROJECT_NAME}_SOURCE_DIR}/patching")

### include directories
include_directories(
  BEFORE
  ${${PROJECT_NAME}_GENERATED_FILES_DIR}
  ${${PROJECT_NAME}_SOURCE_DIR}
  ${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}
  ${${PROJECT_NAME}_SRC_DIR_HTTP_PARSER}
  ${${PROJECT_NAME}_SRC_DIR_PAPP}
  ${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}
)
#
# preparation
################################################################################

################################################################################
# FILES
#
### GENERAL source files
set(${PROJECT_NAME}_HEADERS_GENERAL
  "${${PROJECT_NAME}_SOURCE_DIR}/stdafx.h"
  "${${PROJECT_NAME}_SOURCE_DIR}/targetver.h"
  "${${PROJECT_NAME}_SOURCE_DIR}/dllmain.h"
  "${${PROJECT_NAME}_SOURCE_DIR}/resource.h"
)
source_group("General\\Header Files" FILES ${${PROJECT_NAME}_HEADERS_GENERAL})

set(${PROJECT_NAME}_SOURCES_GENERAL
  "${${PROJECT_NAME}_SOURCE_DIR}/stdafx.cpp"
  "${${PROJECT_NAME}_SOURCE_DIR}/dllmain.cpp"
)
source_group("General\\Source Files" FILES ${${PROJECT_NAME}_SOURCES_GENERAL})

### protocol handler source files
set(${PROJECT_NAME}_HEADERS_PROT_HANDLER
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/ProtocolHandlerRegistrar.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/Buffer.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandler.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandlerClassFactory.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolHandlerClassFactoryT.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolHandlerT.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandler.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandlerClassFactory.h"
)
source_group("Handler\\Header Files" FILES ${${PROJECT_NAME}_HEADERS_PROT_HANDLER})

set(${PROJECT_NAME}_SOURCES_PROT_HANDLER
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/ProtocolHandlerRegistrar.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/Buffer.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandler.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandlerClassFactory.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandler.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandlerClassFactory.cpp"
)
source_group("Handler\\Source Files" FILES ${${PROJECT_NAME}_SOURCES_PROT_HANDLER})

### http parser source files
set(${PROJECT_NAME}_HEADERS_HTTP_PARSER
  "${${PROJECT_NAME}_SRC_DIR_HTTP_PARSER}/http_parser.h"
)
source_group("http-parser\\Header Files" FILES ${${PROJECT_NAME}_HEADERS_HTTP_PARSER})

set(${PROJECT_NAME}_SOURCES_HTTP_PARSER
  "${${PROJECT_NAME}_SRC_DIR_HTTP_PARSER}/http_parser.c"
)
source_group("http-parser\\Source Files" FILES ${${PROJECT_NAME}_SOURCES_HTTP_PARSER})

### papp source files
set(${PROJECT_NAME}_HEADERS_PAPP
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/PassthroughObject.h"
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/ProtocolCF.h"
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/ProtocolImpl.h"
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/SinkPolicy.h"
)
source_group("papp\\Header Files" FILES ${${PROJECT_NAME}_HEADERS_PAPP})

set(${PROJECT_NAME}_SOURCES_PAPP
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/ProtocolCF.inl"
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/ProtocolImpl.inl"
  "${${PROJECT_NAME}_SRC_DIR_PAPP}/SinkPolicy.inl"
)
source_group("papp\\Source Files" FILES ${${PROJECT_NAME}_SOURCES_PAPP})

### protocol patching source files
set(${PROJECT_NAME}_HEADERS_PROT_PATCHING
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Application.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ClassFactories.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/FrameRecord.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/GlobalMap.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Headers.h"
<<<<<<< Updated upstream
=======
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/IDispatchExImpl.h"
>>>>>>> Stashed changes
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/IInternetProtocolCFPatch.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Protocol.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ProtocolSink.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Request.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/RequestRecord.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Response.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Scheme.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ThreadRecord.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/VTablePatch.h"
)
source_group("ProtocolPatch\\Header Files" FILES ${${PROJECT_NAME}_HEADERS_PROT_PATCHING})

set(${PROJECT_NAME}_SOURCES_PROT_PATCHING
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Application.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/FrameRecord.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Headers.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Protocol.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ProtocolLibrary.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ProtocolSink.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Request.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/RequestRecord.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/Response.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCHING}/ThreadRecord.cpp"
)
source_group("ProtocolPatch\\Source Files" FILES ${${PROJECT_NAME}_SOURCES_PROT_PATCHING})

### generated files
set(${PROJECT_NAME}_MIDL_TLB_FILE "${${PROJECT_NAME}_GENERATED_FILES_DIR}/${PROJECT_NAME}.tlb")
set(${PROJECT_NAME}_MIDL_I_H_FILE  "${${PROJECT_NAME}_GENERATED_FILES_DIR}/${PROJECT_NAME}_i.h")
set(${PROJECT_NAME}_MIDL_I_C_FILE  "${${PROJECT_NAME}_GENERATED_FILES_DIR}/${PROJECT_NAME}_i.c")
set(${PROJECT_NAME}_GENERATED_FILES
  "${${PROJECT_NAME}_MIDL_TLB_FILE}"
  "${${PROJECT_NAME}_MIDL_I_H_FILE}"
  "${${PROJECT_NAME}_MIDL_I_C_FILE}"
)
source_group("Generated files" FILES ${${PROJECT_NAME}_GENERATED_FILES})

set_source_files_properties(
  ${${PROJECT_NAME}_MIDL_TLB_FILE}
  ${${PROJECT_NAME}_MIDL_I_H_FILE}
  PROPERTIES
    HEADER_FILE_ONLY 1
    GENERATED 1
)

set_source_files_properties(
  ${${PROJECT_NAME}_MIDL_I_C_FILE}
  PROPERTIES
    GENERATED 1
)

### idl file
set (${PROJECT_NAME}_IDL_FILE
  "${${PROJECT_NAME}_SOURCE_DIR}/${PROJECT_NAME}.idl"
)
set_source_files_properties(
  ${${PROJECT_NAME}_IDL_FILE}
  PROPERTIES
    HEADER_FILE_ONLY 1
)
source_group("General" FILES ${${PROJECT_NAME}_IDL_FILE})

### def file
set (${PROJECT_NAME}_DEF_FILES
  "${${PROJECT_NAME}_SOURCE_DIR}/${PROJECT_NAME}.def"
)
source_group("General" FILES ${${PROJECT_NAME}_DEF_FILES})

### resource files
set (${PROJECT_NAME}_RC_FILES
  ${${PROJECT_NAME}_RC_DIR}/${PROJECT_NAME}.rc
)
source_group("Resources" FILES ${${PROJECT_NAME}_RC_FILES})

###
### all header files
set(${PROJECT_NAME}_HEADERS
  ${${PROJECT_NAME}_HEADERS_GENERAL}
  ${${PROJECT_NAME}_HEADERS_PROT_HANDLER}
  ${${PROJECT_NAME}_HEADERS_HTTP_PARSER}
  ${${PROJECT_NAME}_HEADERS_PAPP}
  ${${PROJECT_NAME}_HEADERS_PROT_PATCHING}
)

###
### all source files
set(${PROJECT_NAME}_SOURCES
  ${${PROJECT_NAME}_SOURCES_GENERAL}
  ${${PROJECT_NAME}_SOURCES_PROT_HANDLER}
  ${${PROJECT_NAME}_SOURCES_HTTP_PARSER}
  ${${PROJECT_NAME}_SOURCES_PAPP}
  ${${PROJECT_NAME}_SOURCES_PROT_PATCHING}
)

### precompiled headers
foreach( src_file ${${PROJECT_NAME}_SOURCES} )
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yustdafx.h"
            )
endforeach( src_file ${${PROJECT_NAME}_SOURCES} )
set_source_files_properties("${${PROJECT_NAME}_SOURCE_DIR}/stdafx.cpp" PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
set_source_files_properties("${${PROJECT_NAME}_SRC_DIR_HTTP_PARSER}/http_parser.c" PROPERTIES COMPILE_FLAGS "")

#
# FILES
################################################################################

################################################################################
# cmake
#

### create folder for generated files
file(MAKE_DIRECTORY ${${PROJECT_NAME}_GENERATED_FILES_DIR})

### _WIN64
if (${CMAKE_CL_64})
  add_definitions(-DWIN64 -D_WIN64)
endif (${CMAKE_CL_64})

### MIDL
set(MIDL_ENV_ARG win32)
if (${CMAKE_CL_64})
  set(MIDL_ENV_ARG win64)
endif (${CMAKE_CL_64})

set(${PROJECT_NAME}_MIDL_ARGUMENTS
                     "${${PROJECT_NAME}_IDL_FILE}"
                     /D $<$<CONFIG:Debug>:"_DEBUG"> $<$<CONFIG:Release>:"NDEBUG"> $<$<CONFIG:RelWithDebInfo>:"NDEBUG">
                     /D "MODULES_HAVE_EXPORT_PROPERTY"
                     /nologo
                     /char signed
                     /env ${MIDL_ENV_ARG}
                     /Oicf
                     /tlb "${${PROJECT_NAME}_MIDL_TLB_FILE}"
                     /h "${${PROJECT_NAME}_MIDL_I_H_FILE}"
                     /iid "${${PROJECT_NAME}_MIDL_I_C_FILE}"
                     /proxy "${${PROJECT_NAME}_MIDL_P_C_FILE}"
                     /error stub_data
)

#
# cmake
################################################################################

################################################################################
# target setup
#
add_library(${PROJECT_NAME} SHARED
  ${${PROJECT_NAME}_HEADERS}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_GENERATED_FILES}
  ${${PROJECT_NAME}_RC_FILES}
  ${${PROJECT_NAME}_DEF_FILES}
  ${${PROJECT_NAME}_IDL_FILE}
)

## set UNICODE
add_definitions(-DUNICODE -D_UNICODE)

## Generating files in pre-build step
add_custom_command(TARGET ${PROJECT_NAME}
                   PRE_BUILD
                   COMMAND midl.exe ${${PROJECT_NAME}_MIDL_ARGUMENTS}
                   WORKING_DIRECTORY "${${PROJECT_NAME}_GENERATED_FILES_DIR}"
)

# Register DLL
# if (NOT ${DISABLE_REGISTRATION})
#   add_custom_command(TARGET ${PROJECT_NAME}
#                        POST_BUILD
#                        COMMAND "regsvr32" ARGS "/s" "\"\$(TargetPath)\""
#                        )
# endif (NOT ${DISABLE_REGISTRATION})

# export variables
set(${PROJECT_NAME}_INC_DIRS ${${PROJECT_NAME}_GENERATED_FILES_DIR} PARENT_SCOPE)

set(${PROJECT_NAME}_LIB_DIR "${${PROJECT_NAME}_BINARY_DIR}"  CACHE PATH "Path to ${PROJECT_NAME} lib directory")

message(STATUS "Configuring ${PROJECT_NAME}")
#
# target setup
################################################################################
