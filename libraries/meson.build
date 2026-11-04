# The Haskell libraries; see tools/build-libraries.

# The platform-dependent parts of the base library, answered by asking the
# C compiler: HsBaseConfig.h for the C side, Cupid/SysConfig.hs for Haskell
mksysconfig = executable('mksysconfig', 'tools/mksysconfig.c', install: false)

build_libraries = find_program('tools/build-libraries')
cupiddir_build = meson.current_build_dir() / 'cupiddir'

libraries_stamp = custom_target('cupid-libraries',
  output: 'libraries.stamp',
  command: [build_libraries,
    meson.project_source_root(), cupiddir_build,
    cupid, mksysconfig, meson.current_build_dir() / '..' / 'src',
    host_machine.system(), '@OUTPUT@'],
  depends: [cupid, mksysconfig],
  depend_files: files('tools/build-libraries', '../src/HsFFI.h'),
  build_by_default: true,
  console: true)

meson.add_install_script('tools/install-libraries', cupiddir_build, cupiddir)
