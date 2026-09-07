man_conf = configuration_data()
man_conf.set('configure_input', 'Generated from cuddles.1.in by meson')
man_conf.set('BINDIR', bindir)
man_conf.set('CUDDLESDIR', cuddlesdir)
man_conf.set('CUDDLESPATH', cuddlespath)
man_conf.set('CUDDLESSUFFIXES', cuddlessuffixes)
cuddles_1 = configure_file(input: 'cuddles.1.in', output: 'cuddles.1', configuration: man_conf)
install_man(cuddles_1)

# The user documentation, as markdown.
install_data('README.md', install_dir: cuddlesdir / 'docs')
foreach section : ['tutorials', 'how-to', 'reference', 'explanation']
  install_subdir(section, install_dir: cuddlesdir / 'docs', exclude_directories: '.claude')
endforeach
