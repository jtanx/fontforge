class Libuninameslist < Formula
  desc "A Library of Unicode names and annotation data"
  homepage "https://github.com/fontforge/libuninameslist"
  url "https://github.com/fontforge/libuninameslist/archive/0.5.20150701.tar.gz"
  sha256 "7b4885eb256ba8f3eeeb017ef197e9ca9571416f33d3ae0ebf370310e821c1c8"

  #bottle do
  #  cellar :any
  #  revision 1
  #  sha256 "b74aa7a260b965d0910c86eff34bb29268efe56d2050063ad21e5261b7767697" => :el_capitan
  #  sha1 "ef44221e7e675704a36b0e0e8a78b350c22f67bf" => :yosemite
  #  sha1 "a1beaa7f9e7d1733fd0ab905c4445c30352e876d" => :mavericks
  #  sha1 "46e386cb03763d3de5b584afd77315ee044bcd2b" => :mountain_lion
  #end

  head do
    url "https://github.com/fontforge/libuninameslist.git"

    depends_on "automake" => :build
    depends_on "autoconf" => :build
    depends_on "libtool" => :build
  end

  def install
    if build.head?
      system "autoreconf", "-i"
      system "automake"
    end

    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end
end
