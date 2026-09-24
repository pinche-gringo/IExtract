python = find_program('python3')

test('IExtract', python,
     args: ['-m', 'unittest', '-v', 'test/test.py'],
     env: {'IEXTRACT': iextract.full_path()},
     workdir: meson.project_source_root(),
     depends: iextract)
