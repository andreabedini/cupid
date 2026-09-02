project('hugs98', 'c',
  version: '2006.09.20',
  license: 'BSD-3-Clause',
  meson_version: '>= 1.1.0',
  default_options: ['c_std=c23', 'buildtype=debugoptimized', 'warning_level=1'])

# The string the banner and :version print
hugs_version_string = 'September 2006'

cc = meson.get_compiler('c')
host = host_machine.system()
is_windows = host == 'windows'

# Operating system name in the spelling the Haskell libraries test for.
# Must agree with src/platform.h.
host_os_names = {
  'linux': 'linux',
  'darwin': 'darwin',
  'windows': 'mingw32',
  'freebsd': 'freebsd',
  'dragonfly': 'freebsd',
  'netbsd': 'netbsd',
  'openbsd': 'openbsd',
}
if host not in host_os_names
  error('Hugs supports Linux, macOS, Windows and the BSDs only (host is @0@)'.format(host))
endif
host_os = host_os_names[host]

# ---------------------------------------------------------------------------
# Install layout
# ---------------------------------------------------------------------------

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
libdir = prefix / get_option('libdir')
datadir = prefix / get_option('datadir')
hugsdir = get_option('hugsdir')
if hugsdir == ''
  hugsdir = libdir / 'hugs'
endif

if is_windows
  hugspath = '.;{Hugs}\\packages\\*'
  hugssuffixes = '.hs;.lhs'
else
  hugspath = '.:{Home}/lib/hugs/packages/*:/usr/local/lib/hugs/packages/*:{Hugs}/packages/*'
  hugssuffixes = '.hs:.lhs'
endif

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

libm = cc.find_library('m', required: false)
libdl = cc.find_library('dl', required: false)

readline_opt = get_option('readline')
readline_dep = dependency('readline', required: false)
if not readline_dep.found() and not readline_opt.disabled()
  rl = cc.find_library('readline', required: false)
  if not rl.found()
    rl = cc.find_library('edit', required: false)
  endif
  if rl.found()
    termcap = []
    foreach name : ['ncurses', 'tinfo', 'termcap', 'curses']
      if termcap.length() == 0
        lib = cc.find_library(name, required: false)
        if lib.found()
          termcap += lib
        endif
      endif
    endforeach
    readline_dep = declare_dependency(dependencies: [rl] + termcap)
  endif
endif
if readline_opt.enabled() and not readline_dep.found()
  error('readline was requested but neither readline nor libedit was found')
endif
use_readline = readline_dep.found() and not readline_opt.disabled()

hp2ps = find_program('hp2ps', required: false)

# The command ffihugs runs to turn generated C into a loadable library
cc_cmd = ' '.join(cc.cmd_array())
if host == 'darwin'
  dll_flags = '-bundle -undefined dynamic_lookup'
elif is_windows
  dll_flags = cc.get_argument_syntax() == 'msvc' ? '/LD' : '-shared'
else
  dll_flags = '-shared -fPIC'
endif
mkdll_cmd = cc_cmd + ' ' + dll_flags

# ---------------------------------------------------------------------------
# config.h: build options and install paths only; platform facts live in
# src/platform.h
# ---------------------------------------------------------------------------

if not get_option('debug-code')
  add_project_arguments('-DNDEBUG=1', language: 'c')
endif

conf = configuration_data()
conf.set_quoted('HUGS_VERSION', hugs_version_string)
conf.set_quoted('HUGSDIR', hugsdir)
conf.set_quoted('HUGSPATH', hugspath)
conf.set_quoted('HUGSSUFFIXES', hugssuffixes)
conf.set_quoted('MKDLL_CMD', mkdll_cmd)
conf.set('USE_PREPROCESSOR', 1)
if use_readline
  conf.set('USE_READLINE', 1)
endif
if get_option('char-encoding') == 'utf8'
  conf.set('CHAR_ENCODING_UTF8', 1)
elif get_option('char-encoding') == 'locale'
  conf.set('CHAR_ENCODING_LOCALE', 1)
endif
if hp2ps.found()
  conf.set('HAVE_HP2PS', 1)
endif

# Macros that are tested with #ifdef as well as #if, so only define the
# ones that are on
option_macros = {
  'profiling': ['PROFILING'],
  'timer': ['WANT_TIMER'],
  'debug-code': ['DEBUG_CODE', 'DEBUG_PRINTER'],
  'tag-checks': ['CHECK_TAGS'],
  'internal-prims': ['INTERNAL_PRIMS', 'BYTECODE_PRIMS'],
  'only98': ['HASKELL_98_ONLY'],
  'stack-dumps': ['GIMME_STACK_DUMPS'],
  'path-canonicalization': ['PATH_CANONICALIZATION'],
  'small-banner': ['SMALL_BANNER'],
  'explain-instance-resolution': ['EXPLAIN_INSTANCE_RESOLUTION'],
  'dont-panic': ['DONT_PANIC'],
}
foreach opt, macros : option_macros
  if get_option(opt)
    foreach macro : macros
      conf.set(macro, 1)
    endforeach
  endif
endforeach

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------

subdir('src')
if get_option('libraries')
  subdir('libraries')
endif
subdir('docs')

install_subdir('demos', install_dir: hugsdir, exclude_files: ['Makefile.in'])
install_data('Readme', 'License', 'Credits', install_dir: hugsdir)

summary({
  'hugsdir': hugsdir,
  'readline': use_readline,
  'character encoding': get_option('char-encoding'),
  'Haskell libraries': get_option('libraries'),
  'ffihugs compile command': mkdll_cmd,
}, section: 'Configuration')
