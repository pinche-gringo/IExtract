iextract_sources = files('Writer.cpp', 'IExtract.cpp')

format_sources = {
  'abiword': files('ParseAbiword.cpp'),
  'gif': files('ParseGIF.cpp'),
  'html': files('ParseHTML.cpp'),
  'jpeg': files('ParseJPG.cpp'),
  'mp3': files('ParseMP3.cpp'),
  'msoffice': files('ParseMSOffice.cpp', 'ParseOOXML.cpp'),
  'ogg': files('ParseOGG.cpp'),
  'oo': files('ParseSOffice.cpp', 'ParseOOffice.cpp'),
  'pdf': files('ParsePDF.cpp'),
  'png': files('ParsePNG.cpp'),
  'rtf': files('ParseRTF.cpp'),
}
foreach format : enabled_formats
  iextract_sources += format_sources[format]
endforeach

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
