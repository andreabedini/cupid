man_conf = configuration_data()
man_conf.set('configure_input', 'Generated from hugs.1.in by meson')
man_conf.set('BINDIR', bindir)
man_conf.set('HUGSDIR', hugsdir)
man_conf.set('HUGSPATH', hugspath)
man_conf.set('HUGSSUFFIXES', hugssuffixes)
hugs_1 = configure_file(input: 'hugs.1.in', output: 'hugs.1', configuration: man_conf)
install_man(hugs_1)

install_data(
  'ffi-notes.txt', 'machugs-notes.txt',
  'winhugs-notes.txt', 'server.html', 'server.tex',
  install_dir: hugsdir / 'docs')
