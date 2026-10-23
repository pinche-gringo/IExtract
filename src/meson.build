iextract_sources = files('Writer.cpp', 'IExtract.cpp', 'ParseXMP.cpp')
if zlib_dep.found()
  iextract_sources += files('ZipArchive.cpp')
endif

# Sources needed for the formats (including the ones shared with other formats)
format_sources = {
  'abiword': ['ParseAbiword.cpp'],
  'epub': ['ParseEPUB.cpp'],
  'flac': ['ParseFLAC.cpp', 'ParseOGG.cpp'],
  'gif': ['ParseGIF.cpp'],
  'html': ['ParseHTML.cpp'],
  'jpeg': ['ParseJPG.cpp', 'ParseTIFF.cpp'],
  'mp3': ['ParseMP3.cpp'],
  'mp4': ['ParseMP4.cpp'],
  'msoffice': ['ParseMSOffice.cpp', 'ParseOOXML.cpp'],
  'ogg': ['ParseOGG.cpp'],
  'oo': ['ParseSOffice.cpp', 'ParseOOffice.cpp'],
  'pdf': ['ParsePDF.cpp'],
  'png': ['ParsePNG.cpp'],
  'rtf': ['ParseRTF.cpp'],
  'svg': ['ParseSVG.cpp'],
  'tiff': ['ParseTIFF.cpp'],
  'webp': ['ParseWebP.cpp', 'ParseTIFF.cpp'],
}
used_sources = []
foreach format : enabled_formats
  foreach source : format_sources[format]
    if source not in used_sources
      used_sources += source
    endif
  endforeach
endforeach
iextract_sources += files(used_sources)

# Generate the code to read/write the options from their declaration
mgeni = find_program('mgeni.pl')
iextract_sources += custom_target('Options.meta',
                                  input: 'Options.h',
                                  output: 'Options.meta',
                                  command: [mgeni, '@INPUT@'],
                                  capture: true)

iextract = executable('IExtract', iextract_sources,
                      include_directories: top_inc,
                      dependencies: deps,
                      install: true)

install_headers('Properties.h', subdir: 'IExtract')

cppcheck = find_program('cppcheck', required: false)
if cppcheck.found()
  run_target('check_code',
             command: [cppcheck, '--force', meson.current_source_dir(),
                       '--enable=all', '-I' + meson.project_build_root()])
endif
