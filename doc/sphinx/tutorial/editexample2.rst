Tutorial #2
===========

.. 
  * :ref:`Font Creation <editexample.FontCreate>`
  * :ref:`Creating a glyph (tracing outlines) <editexample.CharCreate>`
  * :doc:`Create glyph outlines using spiro points <editspiro>`
  * :doc:`Importing a glyph from Inkscape (or Illustrator, or some other vector editor) <importexample>`
  * :ref:`Navigating to other glyphs <editexample2.Navigating>`
  * :ref:`On to the next glyph (consistent directions) <editexample2.Creating-o>`
  * :ref:`Consistent serifs and stem widths <editexample3.consistent-stems>`
  * :ref:`Building accented glyphs <editexample4.accents>`
  * :ref:`Building a ligature <editexample4.ligature>`
  * :ref:`Lookups and features <editexample4.lookups>`
  * :ref:`Examining metrics <editexample5.metrics>`
  * :ref:`Kerning <editexample5.Kerning>`
  * :ref:`Glyph variants <editexample6.Variants>`
  * :ref:`Anchoring marks <editexample6.Marks>`
  * :ref:`Conditional features <editexample6-5.Conditional>`
  * :ref:`Checking your font <editexample7.checking>`
  * :ref:`Generating it <editexample7.generating>`
  * :ref:`Font Families <editexample7.Families>`
  * :ref:`Final Summary <editexample7.summary>`
  * :doc:`Bitmap strikes <editexample8>`
  * :doc:`Scripting Tutorial <scripting-tutorial>`
  * :ref:`Notes on various scripts <scriptnotes.Special>`


.. _editexample2.Navigating:

Navigating to glyphs.
---------------------

The font view provides one way of navigating around the glyphs in a font. Simple
scroll around it until you find the glyph you need and then double click on it
to open a window looking at that glyph.

Typing a glyph will move to that glyph.

However some fonts are huge (Chinese, Japanese and Korean fonts have thousands
or even tens of thousands of glyphs) and scrolling around the font view is a an
inefficient way of finding your glyph. :menuselection:`View --> Goto` provides a
simple dialog which will allow you to move directly to any glyph for which you
know the name (or encoding). If your font is a Unicode font, then this dialog
will also allow you to find glyphs by block name (ie. Hebrew rather than Alef).

The simplest way to navigate is just to go to the next or previous glyph. And
:menuselection:`View --> Next Char` and :menuselection:`View --> Prev Char` will
do exactly that.


.. _editexample2.Creating-o:

Creating the letter "o" -- consistent directions
------------------------------------------------

In the previous example the bitmap of the letter filled the canvas of the image.
And when FontForge imported the image it needed to be scaled once in the
program. But usually when you create the image of the letter you have some idea
of how much white space there should be around it. If your images are exactly
one em high then FontForge will scale them automatically to be the right size.
So in the following examples all the images have exactly the right amount of
white-space around them to fit perfectly in an em.

For the next example double click on the square in the font view that should
contain "o", and import "o_Ambrosia.png" into it.

.. list-table:: Stages in editing "o"

   * - .. image:: /images/o1.png

     - .. image:: /images/o2.png

     - .. image:: /images/o3.png

     - .. image:: /images/o4.png

Notice that the first outline is drawn clockwise and the second
counter-clockwise. This change in drawing direction is important. Both
PostScript and TrueType require that the outer boundary of a glyph be drawn in a
certain direction (they happen to be opposite from each other, which is a mild
annoyance), within FontForge all outer boundaries must be drawn clockwise, while
all inner boundaries must be drawn counter-clockwise.

If you fail to alternate directions between outer and inner boundaries you may
get results like the one on the left

.. image:: /images/o-baddir.png

. If you fail to draw the outer contour in a clockwise fashion the errors are
more subtle, but will generally result in a less pleasing result once the glyph
has been rasterized.

.. note::
  **TECHNICAL AND CONFUSING**

  The exact behavior of rasterizers varies. Early PostScript rasterizers used a
  "non-zero winding number rule" while more recent ones use an "even-odd" rule.
  TrueType uses the "non-zero" rule. The description given above is for the
  "non-zero" rule. The "even-odd" rule would fill the "o" correctly no matter
  which way the paths were drawn (though there would probably be subtle
  problems with hinting).

.. _editexample2.even-odd-non-zero:

Filling using the even-odd rules that a line is drawn from the current pixel to
infinity (in any direction) and the number of contour crossings is counted. If
this number is even the pixel is not filled. If the number is odd the pixel is
filled. In the non-zero winding number rule the same line is drawn, contour
crossings in a clockwise direction add 1 to the crossing count,
counter-clockwise contours subtract 1. If the result is 0 the pixel is not
filled, any other result will fill it.

The command :ref:`Element->Correct Direction <elementmenu.Correct>` will look at
each selected contour, figure out whether it qualifies as an outer or inner
contour and will reverse the drawing direction when the contour is drawn
incorrectly.