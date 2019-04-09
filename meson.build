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

dep_glib = dependency('glib-2.0')
dep_gio = dependency('gio-2.0')
dep_freetype = dependency('freetype2')
dep_libxml2 = dependency('libxml-2.0')
dep_zlib = dependency('zlib')
dep_m = cc.find_library('m', required : false)
dep_gdk = dependency('gdk-3.0', required: false)
dep_libspiro = dependency('libspiro', required: false)
conf_data.set10('FONTFORGE_CAN_USE_GDK', dep_gdk.found())
conf_data.set10('_LIBSPIRO_FUN', dep_libspiro.found())
if not dep_libspiro.found()
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
dep_iconv = declare_dependency(dependencies: iconv_lib)

default_includes = include_directories('.', 'inc')

subdir('inc')
subdir('Unicode')
subdir('gutils')
subdir('gdraw')
subdir('fontforge')
subdir('fontforgeexe')
