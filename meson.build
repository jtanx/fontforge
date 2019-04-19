project('fontforge', 'c',
  version: '20190409',
  default_options: [
    'buildtype=plain',
    'warning_level=1',
    'c_std=gnu99',
    'cpp_std=c++11',
  ],
  meson_version : '>= 0.37.0',
  license: 'GPLv3+'
)

cc = meson.get_compiler('c')

conf_data = configuration_data()
conf_data.set10('HAVE_PTHREAD_H', false)
conf_data.set10('HAVE_ICONV_H', true)
conf_data.set10('FONTFORGE_CAN_USE_GDK', true)
conf_data.set10('X_DISPLAY_MISSING', true)
conf_data.set10('FONTFORGE_CONFIG_USE_DOUBLE', true)

glib_dep = dependency('glib-2.0')
gio_dep = dependency('gio-2.0')
freetype_dep = dependency('freetype2')
libxml2_dep = dependency('libxml-2.0')
zlib_dep = dependency('zlib')
m_dep = cc.find_library('m', required : false)
gdk_dep = dependency('gdk-3.0', required: false)
libspiro_dep = dependency('libspiro', required: false)
conf_data.set10('FONTFORGE_CAN_USE_GDK', gdk_dep.found())
conf_data.set10('_LIBSPIRO_FUN', libspiro_dep.found())
if not libspiro_dep.found()
  conf_data.set10('_NO_LIBSPIRO', true)
endif

iconv_lib = []
if not cc.has_function('iconv_open')
  if cc.has_header_symbol('iconv.h', 'iconv_open') or cc.has_header_symbol('iconv.h', 'libiconv_open')
    iconv_lib = cc.find_library('iconv')
  else
    error('iconv was not found')
  endif
endif
iconv_dep = declare_dependency(dependencies: iconv_lib)

default_includes = include_directories('.', 'inc')

subdir('inc')
subdir('Unicode')
subdir('gutils')
subdir('fontforge')
subdir('gdraw')
subdir('fontforgeexe')
