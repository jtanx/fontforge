
include(TestUtils)

add_executable(systestdriver systestdriver.c)
target_link_libraries(systestdriver PRIVATE GLIB::GLIB)
target_compile_options(systestdriver PRIVATE -Wall -pedantic)

# Generated files
add_generation_target(Ambrosia.pt3 Ambrosia.sfd)
add_generation_target(Ambrosia.woff2 Ambrosia.sfd)

# Downloaded files
add_download_target("NotoSans-Regular.ttc" "https://github.com/fontforge/debugfonts/raw/master/NotoSans-Regular.ttc")
add_download_target("MunhwaGothic-Bold" "https://github.com/fontforge/debugfonts/raw/master/MunhwaGothic-Bold")

add_custom_target(test_dependencies
  DEPENDS
  "${CMAKE_CURRENT_BINARY_DIR}/fonts/Ambrosia.pt3"
  "${CMAKE_CURRENT_BINARY_DIR}/fonts/Ambrosia.woff2"
  "${CMAKE_CURRENT_BINARY_DIR}/fonts/NotoSans-Regular.ttc"
  "${CMAKE_CURRENT_BINARY_DIR}/fonts/MunhwaGothic-Bold"
)

add_custom_target(check
  COMMAND "${CMAKE_CTEST_COMMAND}" "--output-on-failure"
  DEPENDS
    test_dependencies
    fontforgeexe
    systestdriver
  VERBATIM
  USES_TERMINAL
)



# > When running tests, CTest learned to treat skipped tests (using the SKIP_RETURN_CODE property) the same as tests with the new DISABLED property. Due to this change, CTest will not indicate failure when all tests are either skipped or pass.

add_test(NAME bashtest COMMAND bash -c "exit 77")
set_tests_properties(bashtest PROPERTIES DISABLED TRUE)

add_ff_test(test001 test001.pe Caliban.sfd Zapfino-4.0d3.dfont Zapfino-4.1d6.dfont "Generate a few font files")
add_ff_test(test002 test002.pe HanSans-Bold.otf "OTF CID font check")
add_ff_test(test003 test003.pe MunhwaGothic-Bold "Non-OTF CID font check")
add_ff_test(test004 test004.pe mingliu.ttc "Chunk of memory check")
add_ff_test(test005 test005.pe VKern.sfd "Kerning information checks")
add_ff_test(test006 test006.pe mingliu.ttc GillSans.dfont ItalicHand.sfd ItalicHand.svg MunhwaGothic-Bold Caliban.otf "Incorrect names and count checks")
add_ff_test(test007 test007.pe FormalScript.sfd dvng10a-crash.sfd "Contextual chaining tables don't crash")
add_ff_test(test008 test008.pe n019003l.pfb "Contextual chaining tables don't crash")
add_ff_test(test009 test009.pe Ambrosia.pt3 "Read and generate Type3 fonts")
add_ff_test(test010 test010.pe GillSans.dfont "Generate some error messages")
add_ff_test(test101 test101.pe "Check for unexpected")
add_ff_test(test102 test102.pe AHBugs.sfd "Infinite loop test on 0 length spline")
add_ff_test(test103 test103.pe "Copy/Paste clipboard test for crash")
add_ff_test(test104 test104.pe SplineOverlapBug1.sfd "Remove overlap test #1")
add_ff_test(test105 test105.pe "while loop check  loop 10x")
add_ff_test(test106 test106.pe OmittedCharBugs.sfd "Simple 'Bad Kerning' check")
add_ff_test(test107 test107.pe Ambrosia.sfd AmbrosiaBold.sfd AmbrosiaItalic.sfd "Generate a font family") # also calls 'helper107.pe'
add_ff_test(test108 test108.pe ItalicHand.sfd "Make GPOS  GSUB  morx without crashing")
add_ff_test(test109 test109.pe OverlapBugs.sfd feta20.pfb "Remove overlap test #2")
add_ff_test(test110 test110.pe SimplifyBugs.sfd "Test simplify")
add_ff_test(test111 test111.pe cmbsy10.pfb "Test merge fonts")
add_ff_test(test112 test112.pe CourierNew.ttf "Test quadratic conversion")
add_ff_test(test113 test113.pe CourierNew.ttf "Count contour points properly")
add_ff_test(test114 test114.pe CaslonMM.sfd "Generate  save and open a .pfa file")
add_ff_test(test115 test115.pe n019003l.pfb "Replace with reference")
add_ff_test(test116 test116.pe merged.cff "Merged")
add_ff_test(test117 test117.pe ecrm1000.pfb ecrm1000.tfm "Merge kern")
add_ff_test(test118 test118.pe NimbusLGCUni-Regular.sfd "Test Unicode encodings") # also uses helper118A.pe helper118B.pe
add_ff_test(test119 test119.pe DejaVuSerif.sfd "The 'i' glyph test")
add_ff_test(test120 test120.pe NumberPoints.sfd "Another deja-vu point number")
add_ff_test(test121 test121.pe AddExtremumTest.sfd "Don't distort the curve (by much)")
#add_ff_test(test122 test122.pe Hinting.sfd "Check AutoHint works and retains hints")
add_ff_test(test123 test123.pe MinionPro-Regular.otf "Produce equivalent font in CFF check")
add_ff_test(test124 test124.pe "Check that detach glyph works #1")
add_ff_test(test125 test125.pe MinionPro-Regular.fea "Check that detach glyph works #2")
add_ff_test(test126 test126.pe DataURI.sfd "Try SVG and PT3")
add_ff_test(test127 test127.pe Ambrosia.sfd "Test generation of TrueType instructions")
#add_ff_test(svg2ttf svg2ttf.pe "")
add_ff_test(test128 test128.pe "Test StrSplit overrun")
add_ff_test(test129 test129.pe "Ligatures and Fractions Table Lookups")
add_ff_test(test130 test130.pe NotoSans-Regular.ttc "TTC loading")
add_ff_test(test131 test131.pe "libnameslist checks")
add_ff_test(test132 test132.pe Ambrosia.woff2 "WOFF2 round tripping")
add_ff_test(test133 test133.pe test133.fea "Allowed characters in glyph class names")
add_ff_test(test134 test134.pe test134.fea "Short-hand contextual multiple substitution")
add_ff_test(test135 test135.pe test135.fea "GDEF glyph classes")
add_ff_test(test136 test136.pe test136.fea "Re-using mark filtering set")
add_ff_test(test137 test137.pe Ambrosia.sfd "file:// protocol")
add_ff_test(test138 test138.pe "Array sanity checking")

add_test(NAME pytestrun
  COMMAND "${CMAKE_COMMAND}"
    -D "TEST_SCRIPT:STRING=${CMAKE_CURRENT_SOURCE_DIR}/test0001.py"
    -D "TEST_EXECUTABLE:FILEPATH=$<TARGET_FILE:fontforgeexe>"
    -D "GEN_FONTS_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/fonts"
    -D "SRC_FONTS_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fonts"
    -D "TEST_INPUTS:LIST=Ambrosia.sfd"
    -D "TEST_LANG:STRING=py"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/testing/TestFixture.cmake"
  WORKING_DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_test(NAME pytestrunhook
  COMMAND "${CMAKE_COMMAND}"
    -D "TEST_SCRIPT:STRING=${CMAKE_CURRENT_SOURCE_DIR}/test0001.py"
    -D "TEST_EXECUTABLE:FILEPATH=${Python3_EXECUTABLE}"
    -D "GEN_FONTS_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/fonts"
    -D "SRC_FONTS_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fonts"
    -D "HOOK_DIR:PATH=$<TARGET_FILE_DIR:fontforge_pyhook>"
    -D "TEST_INPUTS:LIST=Ambrosia.sfd"
    -D "TEST_LANG:STRING=pyhook"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/testing/TestFixture.cmake"
  WORKING_DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}"
)

#add_test(NAME setup_test_dependencies COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target test_dependencies)
#add_test(NAME test1
#  COMMAND bash -c "echo >&2 JAJJA"
#  VERBATIM
#)
#set_property(TEST test1 PROPERTY SKIP_RETURN_CODE 77)

#set_property(TEST test1 PROPERTY REQUIRED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Caliban.sfd" "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Zapfino-4.0d3.dfont" "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Zapfino-4.1d6.dfont")
#add_test(NAME test2-a-v
#  COMMAND fontforgeexe -lang=ff -script "${CMAKE_CURRENT_SOURCE_DIR}/test003.pe" "${CMAKE_CURRENT_BINARY_DIR}/fonts/MunhwaGothic-Bold"
#  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/test2")

# Use to control when a test runs
# https://stackoverflow.com/questions/50468412/ctest-how-to-skip-other-tests-if-the-precheck-test-fails
# https://stackoverflow.com/questions/54525943/skipping-a-ctest-if-the-target-was-not-built
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests/10824578#10824578
#REQUIRED_FILES

#add_dependencies(test test_dependencies)