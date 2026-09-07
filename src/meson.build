config_h = configure_file(output: 'config.h', configuration: conf)

# Operating system and architecture names for platform.h; the vendor field
# only matters for the platform string
platform_conf = configuration_data()
platform_conf.set('configure_input', 'Generated from platform.h.in by meson')
platform_conf.set('HOST_OS', host_os)
platform_conf.set('HOST_ARCH', host_machine.cpu_family())
host_vendor = host == 'darwin' ? 'apple' : 'unknown'
platform_conf.set('HOST_PLATFORM_CPP',
  '@0@_@1@_@2@'.format(host_machine.cpu_family(), host_vendor, host_os))
platform_h = configure_file(input: 'platform.h.in', output: 'platform.h',
  configuration: platform_conf)
src_inc = include_directories('.')

# These five files allocate heap cells and rely on the conservative
# collector finding every live cell on the C stack.  An optimiser may keep
# a pointer to a cell as a common subexpression instead of the cell itself,
# which the collector's scan of the stack would miss, so they are built
# without optimisation.  The evaluator files are safe because they do not
# assume conservative collection (see the notice in builtin.c).
gc_sensitive = files('compiler.c', 'module.c', 'subst.c', 'static.c', 'type.c')

core = files(
  'builtin.c', 'char.c', 'errors.c', 'evaluator.c', 'ffi.c', 'goal.c',
  'input.c', 'machdep.c', 'machine.c', 'opts.c', 'output.c',
  'script.c', 'storage.c', 'strutil.c', 'version.c',
)

no_opt = cc.get_argument_syntax() == 'msvc' ? '/Od' : '-O0'
libgc = static_library('cuddles-gcsafe', gc_sensitive,
  c_args: no_opt, include_directories: src_inc)
libcore = static_library('cuddles-core', core, include_directories: src_inc)

# The two groups reference each other, so link them as objects rather than
# as archives to avoid link-order problems
core_objs = [
  libcore.extract_all_objects(recursive: true),
  libgc.extract_all_objects(recursive: true),
]

deps = [libm, libdl]
if use_readline
  deps += readline_dep
endif

cuddles = executable('cuddles', files('cuddles.c', 'edit.c'),
  objects: core_objs, include_directories: src_inc, dependencies: deps,
  install: true)

runcuddles = executable('runcuddles', files('runcuddles.c', 'server.c'),
  objects: core_objs, include_directories: src_inc, dependencies: deps,
  install: true)

fficuddles = executable('fficuddles', files('runcuddles.c', 'server.c'),
  c_args: '-DFFI_COMPILER',
  objects: core_objs, include_directories: src_inc, dependencies: deps,
  install: true)

install_headers('HsFFI.h', install_dir: cuddlesdir / 'include')
