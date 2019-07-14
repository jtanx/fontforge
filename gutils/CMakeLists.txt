add_library(gutils OBJECT
  dlist.c
  fsys.c
  gcol.c
  g_giomime.c
  gimagebmpP.h
  gimage.c
  gimagereadbmp.c
  gimageread.c
  gimagereadgif.c
  gimagereadjpeg.c
  gimagereadpng.c
  gimagereadras.c
  gimagereadrgb.c
  gimagereadtiff.c
  gimagereadxbm.c
  gimagereadxpm.c
  gimagewritebmp.c
  gimagewritegimage.c
  gimagewritejpeg.c
  gimagewritepng.c
  gimagewritexbm.c
  gimagewritexpm.c
  gio.c
  giofile.c
  giofuncP.h
  giotrans.c
  gutils.c
  gwwintl.c
  prefs.c
  prefs.h
  unicodelibinfo.c
  unicodelibinfo.h
)

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  target_include_directories(gutils
    PUBLIC
      ${FONTFORGE_COMMON_INCLUDE_DIRS}
    PRIVATE
      ${GIF_INCLUDE_DIR}
      ${GLIB_INCLUDE_DIRS}
      ${JPEG_INCLUDE_DIR}
      ${Libuninameslist_INCLUDE_DIRS}
      ${PNG_INCLUDE_DIRS}
      ${TIFF_INCLUDE_DIRS}
  )
else()
  target_link_libraries(gutils
    PUBLIC
      fontforge_common_headers
    PRIVATE
      gunicode
      GLIB::GLIB
  )
  target_link_libraries_auto(ENABLE_LIBUNINAMESLIST gutils PRIVATE Libuninameslist::Libuninameslist)
  target_link_libraries_auto(ENABLE_LIBPNG          gutils PRIVATE PNG::PNG)
  target_link_libraries_auto(ENABLE_LIBGIF          gutils PRIVATE GIF::GIF)
  target_link_libraries_auto(ENABLE_LIBTIFF         gutils PRIVATE TIFF::TIFF)
  target_link_libraries_auto(ENABLE_LIBJPEG         gutils PRIVATE JPEG::JPEG)
endif()
