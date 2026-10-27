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

# parser.c and parser.h are generated from the bison grammar parser.y, into
# the build directory, so the grammar is the only copy of itself in the
# tree.  They must not also exist in src/: a `#include "parser.h"' in
# src/input.c would find that one first and could be compiled against a
# stale set of token numbers.
# win_bison is WinFlexBison's, the bison for Windows that needs no Unix tools
bison = find_program('bison', 'win_bison')
parser = custom_target('parser',
  input: 'parser.y',
  output: ['parser.c', 'parser.h'],
  command: [bison, '--defines=@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@'])

# These files allocate heap cells and rely on the conservative
# collector finding every live cell on the C stack.  An optimiser may keep
# a pointer to a cell as a common subexpression instead of the cell itself,
# which the collector's scan of the stack would miss, so they are built
# without optimisation.  The evaluator files are safe because they do not
# assume conservative collection (see the notice in builtin.c).
gc_sensitive = files('binding.c', 'class.c', 'compiler.c', 'depend.c',
  'deriving.c', 'kind.c', 'module.c', 'pattern.c', 'preds.c', 'scc.c',
  'static.c', 'subst.c', 'tycon.c', 'type.c', 'typeexp.c')

core = files(
  'char.c', 'errors.c', 'evaluator.c', 'ffi.c', 'fixity.c', 'goal.c',
  'input.c', 'machdep.c', 'machine.c', 'opts.c', 'output.c',
  'script.c', 'storage.c', 'strutil.c', 'tagcheck.c', 'unitable.c',
  'version.c',
  'prims/array.c', 'prims/bignums.c', 'prims/builtin.c', 'prims/dirprim.c',
  'prims/iomonad.c', 'prims/printer.c', 'prims/timeprim.c',
)

no_opt = cc.get_argument_syntax() == 'msvc' ? '/Od' : '-O0'
libgc = static_library('cupid-gcsafe', gc_sensitive,
  c_args: no_opt, include_directories: src_inc)
# input.c includes the line-editing header, so the core library needs the
# dependency's include path; the programs below link the library itself
libcore = static_library('cupid-core', core, parser,
  include_directories: src_inc,
  dependencies: use_readline ? [readline_dep] : [])

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

# The C stack of the main thread is fixed when the program is linked on
# Windows, 2 MB by default with MinGW-w64; give it the 8 MB that Linux and
# macOS give, so a program recurses as deep on every platform.
link_args = is_windows ? ['-Wl,--stack,8388608'] : []

cupid = executable('cupid', files('cupid.c', 'edit.c', 'run.c'),
  objects: core_objs, include_directories: src_inc, dependencies: deps,
  link_args: link_args, install: true)

install_headers('HsFFI.h', install_dir: cupiddir / 'include')
