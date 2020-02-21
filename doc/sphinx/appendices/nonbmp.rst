Non-BMP Unicode bitmap fonts supported by FontForge
===================================================

Ironically, FontForge is stuck in the dark ages when it comes to using fonts. It
should be using libXft and gtk to get full unicode support.

It doesn't. It continues to use the old X bitmap interface which the X
consortium isn't interested in extending beyond BMP (since libXft/gtk do this
well and there is no point in redoing that for an obsolete interface).

But I'd like to draw non-BMP characters at least in my font view.

To this end I have made FontForge recognize some non-standard encodings: ::

   -*-*-*-*-*--*-*-*-*-*-*-UnicodePlane-{1,2,3,...16}

::

   -fontforge-planeone-medium-r-normal--13-120-75-75-p-101-unicodeplane-1

Is a font that fontforge will interpret as containing glyphs from plane 1
(Supplementary Multilingual Plane) -- starting with Linear B glyphs.

As a proof of concept I've created a bdf font containing all the Linear B
syllables, and not much of anything else (I am aware that the CODE200? fonts
provide good glyphs for the non-bmp characters, but they are shareware so I
can't just rasterize them and redistribute them).

If anyone would like to submit glyphs to this font (or to putative similar ones
for planes 2&14) I'll try and coordinate that process.

* :download:`PlaneOne.sfd <PlaneOne.sfd.bz2>`
* :download:`PlaneOne-13.bdf <PlaneOne-13.bdf.bz2>`
* :download:`toplane.c <toplane.c.bz2>`

The sfd file is encoded with a "full unicode" encoding. The program toplane.c
will take a bdf containing one Unicode plane, figure out which plane it is, and
reencode the font with the appropriate UnicodePlane mapping.