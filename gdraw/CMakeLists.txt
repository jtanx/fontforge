
#not included
#  gdrawwacomdriver.c genkeysym.c growcol.c

if(APPLE)
  set(OBJC_SOURCE ggdkcocoa.m)
endif()

add_library(gdraw OBJECT
  choosericons.c
  colorP.h
  ctlvalues.c
  div_tables.c
  drawboxborder.c
  fontP.h
  gaskdlg.c
  gbuttons.c
  gchardlg.c
  gcolor.c
  gcontainer.c
  gdrawable.c
  gdrawbuildchars.c
  gdraw.c
  gdrawerror.c
  gdrawgimage.c
  gdrawP.h
  gdrawtxt.c
  gdrawtxtinit.c
  gfilechooser.c
  gfiledlg.c
  ggadgetP.h
  ggadgets.c
  ggdkcdraw.c
  ggdkdraw.c
  ggdkdrawlogger.c
  ggdkdrawP.h
  ggroupbox.c
  ghvbox.c
  gimageclut.c
  gimagecvt.c
  gimagepsdraw.c
  gimagewriteeps.c
  gimagexdraw.c
  gkeysym.c
  glist.c
  gmatrixedit.c
  gmenu.c
  gprogress.c
  gpsdraw.c
  gpsdrawP.h
  gpstxtinit.c
  gradio.c
  gresedit.c
  gresource.c
  gresourceimage.c
  gresourceP.h
  gsavefiledlg.c
  gscrollbar.c
  gspacer.c
  gtabset.c
  gtextfield.c
  gtextinfo.c
  gwidgetP.h
  gwidgets.c
  gxcdraw.c
  gxcdrawP.h
  gxdraw.c
  gxdrawP.h
  hotkeys.c
  hotkeys.h
  xkeysyms_unicode.c

  ${OBJC_SOURCE}
)

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  target_include_directories(gdraw
    PUBLIC
      ${FONTFORGE_COMMON_INCLUDE_DIRS}
    PRIVATE
      ${GLIB_INCLUDE_DIRS}
      ${GDK3_INCLUDE_DIRS}
      ${X11_INCLUDE_DIR}
  )
else()
  target_link_libraries(gdraw
    PUBLIC
      fontforge_common_headers
    PRIVATE
      gunicode
      gutils
      GLIB::GLIB
  )

  if(ENABLE_GDK)
    target_link_libraries(gdraw PRIVATE GDK3::GDK3)
  else()
    target_link_libraries(gdraw PRIVATE X11::X11)
  endif()
endif()
