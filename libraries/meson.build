# The Haskell libraries.  Building them is a three-stage bootstrap that
# runs Hugs itself (see tools/bootstrap), so it is one custom target that
# depends on the interpreter binaries.

# HsBaseConfig.h for the base package: the one platform question that
# really varies between targets, answered by asking the compiler
mkhsbaseconfig = executable('mkHsBaseConfig', '../packages/base/mkHsBaseConfig.c',
  install: false)
hsbaseconfig_h = custom_target('HsBaseConfig.h',
  output: 'HsBaseConfig.h',
  command: [mkhsbaseconfig],
  capture: true)

bootstrap = find_program('tools/bootstrap')
python = find_program('python3')
stage_dir = meson.current_build_dir() / 'stage'

libraries_stamp = custom_target('hugs-libraries',
  output: 'libraries.stamp',
  command: [bootstrap,
    meson.project_source_root(), stage_dir,
    hugs, runhugs, ffihugs, hsbaseconfig_h, platform_h,
    python, host_os, prefix, '@OUTPUT@'],
  depends: [hugs, runhugs, ffihugs],
  depend_files: files(
    'tools/make_bootlib', 'tools/convert_libraries',
    'tools/hscpp.py',
    'include/ghcconfig.h', 'include/MachDeps.h',
    '../src/HsFFI.h',
  ),
  build_by_default: true,
  console: true)

meson.add_install_script('tools/install-libraries', stage_dir, hugsdir, bindir)
