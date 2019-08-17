# Distributed under the original FontForge BSD 3-clause license

# not included
#  gdrawwacomdriver.c genkeysym.c growcol.c

add_library(gdraw STATIC
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
  ggtkdraw.c
  ggtkdrawP.h
  ghvbox.c
  gimageclut.c
  gimagecvt.c
  gimagexdraw.c
  gkeysym.c
  glist.c
  gmatrixedit.c
  gmenu.c
  gprogress.c
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
  xkeysyms_unicode.c
)

target_link_libraries(gdraw PRIVATE fontforge)

if(ENABLE_X11)
  target_link_libraries(gdraw PUBLIC
    X11::X11
    X11::Xi
    X11::Xkb
    Cairo::Cairo
    Pango::Pango
  )
elseif(ENABLE_GTK)
  set(gdraw_LIBRARIES GTK3::GTK3)
  if(CMAKE_COMPILER_IS_GNUCC)
    set_source_files_properties(ggtkdraw.c PROPERTIES COMPILE_FLAGS "-Wall -Wno-format-zero-length")
  endif()
else()
  if(APPLE)
    target_sources(gdraw PRIVATE ggdkcocoa.m)
  endif()
  target_link_libraries(gdraw PUBLIC GDK3::GDK3)
endif()
