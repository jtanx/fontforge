
foreach(LANG de el en_GB es fr hr it ja ko ml pl pt ru uk vi zh_CN zh_TW)
  gettext_process_po_files(${LANG} ALL PO_FILES ${LANG}.po)
endforeach()

if(ENABLE_MAINTAINER_TOOLS)
  add_executable(toengb toengb.c)
endif()
