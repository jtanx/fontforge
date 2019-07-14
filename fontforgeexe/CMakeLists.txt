add_executable(fontforgeexe main.c)

if(ENABLE_GUI)
  target_sources(fontforgeexe PRIVATE
    alignment.c
    anchorsaway.c
    autowidth2dlg.c
    basedlg.c
    bdfinfo.c
    bitmapdlg.c
    bitmapview.c
    charinfo.c
    charview_private.h
    charview.c
    clipui.c
    combinations.c
    contextchain.c
    cursors.c
    cvaddpoints.c
    cvdebug.c
    cvdgloss.c
    cvexportdlg.c
    cvfreehand.c
    cvgetinfo.c
    cvgridfit.c
    cvhand.c
    cvhints.c
    cvimportdlg.c
    cvknife.c
    cvpalettes.c
    cvpointer.c
    cvruler.c
    cvshapes.c
    cvstroke.c
    cvtranstools.c
    deltaui.c
    displayfonts.c
    effectsui.c
    encodingui.c
    fontinfo.c
    fontview.c
    freetypeui.c
    fvfontsdlg.c
    fvmetricsdlg.c
    gotodlg.c
    groupsdlg.c
    histograms.c
    images.c
    justifydlg.c
    kernclass.c
    layer2layer.c
    lookupui.c
    macencui.c
    math.c
    metricsview.c
    mmdlg.c
    nonlineartransui.c
    openfontdlg.c
    prefs.c
    problems.c
    pythonui.c
    savefontdlg.c
    scriptingdlg.c
    scstylesui.c
    searchview.c
    sftextfield.c
    sfundo.c
    showatt.c
    simplifydlg.c
    splashimage.c
    startui.c
    statemachine.c
    tilepath.c
    transform.c
    ttfinstrsui.c
    uiutil.c
    usermenu.c
    windowmenu.c
    wordlistparser.c
    wordlistparser.h
  )

  if(APPLE)
    target_sources(fontforgeexe PRIVATE macobjective.m)
  endif()

  if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
    target_sources(fontforgeexe PRIVATE $<TARGET_OBJECTS:gdraw>)
    target_link_libraries(fontforgeexe PRIVATE ${gdraw_LIBRARIES} GLIB::GLIB ${GDK3_LIBRARIES})
  else()
    target_link_libraries(fontforgeexe PRIVATE gdraw)
  endif()
else()
  target_sources(fontforgeexe PRIVATE startnoui.c)
endif()

target_link_libraries(fontforgeexe PRIVATE fontforge)
set_target_properties(fontforgeexe PROPERTIES OUTPUT_NAME "fontforge")
