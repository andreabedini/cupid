man_conf = configuration_data()
man_conf.set('configure_input', 'Generated from cupid.1.in by meson')
man_conf.set('BINDIR', bindir)
man_conf.set('CUPIDDIR', cupiddir)
man_conf.set('CUPIDPATH', cupidpath)
man_conf.set('CUPIDSUFFIXES', cupidsuffixes)
cupid_1 = configure_file(input: 'cupid.1.in', output: 'cupid.1', configuration: man_conf)
install_man(cupid_1)

# The user documentation, as markdown.
install_data('README.md', install_dir: cupiddir / 'docs')
foreach section : ['tutorials', 'how-to', 'reference', 'explanation']
  install_subdir(section, install_dir: cupiddir / 'docs', exclude_directories: '.claude')
endforeach
