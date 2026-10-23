project('IExtract', 'cpp',
        version: '0.9.50',
        license: 'GPL-2.0-or-later',
        meson_version: '>= 1.5.0',
        default_options: ['cpp_std=c++26', 'warning_level=2',
                          'buildtype=debugoptimized'])

package_name = 'Extract'
bugreport = 'pelotudo.gringo@gmail.com'
version = meson.project_version()
version_parts = version.split('.')
release = '@0@.@1@'.format(version_parts[0], version_parts[1])

cxx = meson.get_compiler('cpp')

prefix = get_option('prefix')
localedir = get_option('localedir')

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_NAME', package_name)
conf.set_quoted('PACKAGE_TARNAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', version)
conf.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + version)
conf.set_quoted('PACKAGE_BUGREPORT', bugreport)
conf.set_quoted('VERSION', version)
conf.set_quoted('RELEASE', release)
conf.set_quoted('LOCALEDIR', prefix / localedir)

add_project_arguments('-D_GNU_SOURCE', '-Woverloaded-virtual', '-Wsuggest-override',
                      language: 'cpp')

debug_level = get_option('debug-level')
if debug_level >= 0
  add_project_arguments('-g', '-DTRACELEVEL=@0@'.format(debug_level), language: 'cpp')
endif
check_level = get_option('checks')
if check_level >= 0
  add_project_arguments('-DCHECK=@0@'.format(check_level), language: 'cpp')
endif

# Dependencies
deps = [dependency('YGP', version: '>= 0.9.80', include_type: 'system'),
        dependency('boost', include_type: 'system')]
if not cxx.has_header('boost/tokenizer.hpp', dependencies: deps)
  error('Required header boost/tokenizer.hpp not found')
endif

zlib_dep = dependency('zlib', required: false)
conf.set('HAVE_LIBZ', zlib_dep.found())
if zlib_dep.found()
  deps += zlib_dep
endif

intl_dep = dependency('intl', required: get_option('nls'))
nls = intl_dep.found() and find_program('msgfmt', required: get_option('nls')).found()
conf.set('ENABLE_NLS', nls)
conf.set('HAVE_GETTEXT', nls)
if nls
  deps += intl_dep
endif

if get_option('threads')
  deps += dependency('threads')
  add_project_arguments('-DENABLE_THREADS', language: 'cpp')
endif
if get_option('plugins')
  # Plug-ins are loaded with Boost.DLL (using std::filesystem)
  deps += dependency('dl', required: false)
  add_project_arguments('-DENABLE_PLUGINS', '-DBOOST_DLL_USE_STD_FS', language: 'cpp')
endif

# Supported file types
formats = ['html', 'jpeg', 'png', 'gif', 'tiff', 'webp', 'svg', 'mp3', 'ogg',
           'flac', 'mp4', 'pdf', 'oo', 'abiword', 'rtf', 'msoffice', 'epub']
# Formats which can only be handled with the zlib
zlib_formats = ['epub']
enabled_formats = []
foreach format : formats
  enabled = get_option(format) and (zlib_dep.found() or format not in zlib_formats)
  conf.set('SUPPORT_' + format.to_upper(), enabled)
  if enabled
    enabled_formats += format
  endif
endforeach
conf.set('SUPPORT_OOXML', get_option('msoffice') and zlib_dep.found())

configure_file(input: 'IExtract-cfg.h.meson',
               output: 'IExtract-cfg.h',
               configuration: conf)
top_inc = include_directories('.')

subdir('src')
if nls
  subdir('po')
endif
subdir('test')

summary({'Threads': get_option('threads'),
         'Plugins': get_option('plugins'),
         'NLS': nls,
         'OOXML (zlib)': conf.get('SUPPORT_OOXML')}, section: 'Features')
summary({'Formats': enabled_formats}, section: 'Features')
