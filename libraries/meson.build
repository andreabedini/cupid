# The Haskell libraries.  The sources under libraries/ are plain Haskell
# that Cuddles owns; building them means copying them into
# the build directory, generating the platform-dependent files, and running
# fficuddles over the modules with foreign imports.  See tools/build-libraries.

# The platform-dependent parts of the base library, answered by asking the
# C compiler: HsBaseConfig.h for the C side, Cuddles/SysConfig.hs for Haskell
mksysconfig = executable('mksysconfig', 'tools/mksysconfig.c', install: false)

build_libraries = find_program('tools/build-libraries')
cuddlesdir_build = meson.current_build_dir() / 'cuddlesdir'

libraries_stamp = custom_target('cuddles-libraries',
  output: 'libraries.stamp',
  command: [build_libraries,
    meson.project_source_root(), cuddlesdir_build,
    fficuddles, mksysconfig, meson.current_build_dir() / '..' / 'src',
    '@OUTPUT@'],
  depends: [fficuddles, mksysconfig],
  depend_files: files('tools/build-libraries', '../src/HsFFI.h'),
  build_by_default: true,
  console: true)

meson.add_install_script('tools/install-libraries', cuddlesdir_build, cuddlesdir)
