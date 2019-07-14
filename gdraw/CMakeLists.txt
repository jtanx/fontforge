
#not included
#  gdrawwacomdriver.c genkeysym.c growcol.c

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
)

if(APPLE)
  target_sources(gdraw PRIVATE ggdkcocoa.m)
endif()

if(ENABLE_GDK)
  set_available_targets(gdraw_LIBRARIES GDK3::GDK3)
else()
  set_available_targets(gdraw_LIBRARIES
    X11::X11
    X11::Xi
    X11::Xkb
    Cairo::Cairo
    Pango::Pango
  )
endif()

# Required for backwards compatibility
set(gdraw_LIBRARIES ${gdraw_LIBRARIES} PARENT_SCOPE)

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
      gutils
      gunicode
      ${gdraw_LIBRARIES}
  )
endif()
