project('cupid', 'c',
  version: '0.1.0',
  license: 'BSD-3-Clause',
  meson_version: '>= 1.1.0',
  # c2x is C23 under the name it had before ratification: gcc and clang
  # have accepted it since 2019, and meson 1.3 (Ubuntu 24.04) rejects c23.
  default_options: ['c_std=c2x', 'buildtype=debugoptimized', 'warning_level=1'])

# The string the banner and :version print.  The release task sets the
# version above and nowhere else.
cupid_version_string = meson.project_version()

cc = meson.get_compiler('c')
host = host_machine.system()
is_windows = host == 'windows'

# Operating system name in the spelling the Haskell libraries test for;
# it is written into the generated src/platform.h.
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
  error('Cupid supports Linux, macOS, Windows and the BSDs only (host is @0@)'.format(host))
endif
host_os = host_os_names[host]

# ---------------------------------------------------------------------------
# Install layout
# ---------------------------------------------------------------------------

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
libdir = prefix / get_option('libdir')
datadir = prefix / get_option('datadir')
cupiddir = get_option('cupiddir')
# The library directory relative to the directory of the executables, with
# which an installation that has been moved finds its libraries; see
# cupiddir() in src/machdep.c.  Only for the default library directory, and
# only when the executables and the libraries are both under the prefix.
cupiddir_from_bindir = ''
if cupiddir == ''
  cupiddir = libdir / 'cupid'
  fs = import('fs')
  if not fs.is_absolute(get_option('bindir')) and not fs.is_absolute(get_option('libdir'))
    foreach part : get_option('bindir').split('/')
      if part != '' and part != '.'
        cupiddir_from_bindir += '../'
      endif
    endforeach
    cupiddir_from_bindir += get_option('libdir') / 'cupid'
  endif
endif

if is_windows
  # Doubled: the string becomes a C string literal
  cupidpath = '.;{Cupid}\\\\packages\\\\*'
else
  cupidpath = '.:{Home}/lib/cupid/packages/*:/usr/local/lib/cupid/packages/*:{Cupid}/packages/*'
endif
cupidsuffixes = '.hs'

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

libm = cc.find_library('m', required: false)
libdl = cc.find_library('dl', required: false)

readline_opt = get_option('readline')
# Static on Windows, where nothing else would find the DLL next to an
# installed cupid.exe; MSYS2's libedit is WinEditLine, for the console.
readline_dep = dependency('readline', required: false, static: is_windows)
if not readline_dep.found() and not readline_opt.disabled()
  rl = cc.find_library('readline', required: false, static: is_windows)
  if not rl.found()
    rl = cc.find_library('edit', required: false, static: is_windows)
  endif
  if rl.found()
    termcap = []
    # The Windows console needs no terminal database
    foreach name : is_windows ? [] : ['ncurses', 'tinfo', 'termcap', 'curses']
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
# GNU readline and libedit put the header in different places
readline_h = ''
if use_readline
  foreach h : ['readline/readline.h', 'editline/readline.h']
    if readline_h == '' and cc.has_header(h, dependencies: readline_dep)
      readline_h = h
    endif
  endforeach
  if readline_h == ''
    error('a readline library was found but neither readline/readline.h nor editline/readline.h')
  endif
endif

# hp2ps runs where Cupid runs, so a cross build cannot look for it here
have_hp2ps = not meson.is_cross_build() and find_program('hp2ps', required: false).found()

# The command fficupid runs to turn generated C into a loadable library
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
# src/sysdeps.h and the generated src/platform.h
# ---------------------------------------------------------------------------

if not get_option('checks')
  add_project_arguments('-DNDEBUG=1', language: 'c')
endif

conf = configuration_data()
conf.set_quoted('CUPID_VERSION', cupid_version_string)
conf.set_quoted('CUPIDDIR', cupiddir)
if cupiddir_from_bindir != '' and not is_windows
  conf.set_quoted('CUPIDDIR_FROM_BINDIR', cupiddir_from_bindir)
endif
conf.set_quoted('CUPIDPATH', cupidpath)
conf.set_quoted('CUPIDSUFFIXES', cupidsuffixes)
conf.set_quoted('MKDLL_CMD', mkdll_cmd)
conf.set('USE_PREPROCESSOR', 1)
if use_readline
  conf.set('USE_READLINE', 1)
  conf.set_quoted('READLINE_H', readline_h)
  if readline_h == 'readline/readline.h'
    conf.set_quoted('READLINE_HISTORY_H', 'readline/history.h')
  endif
endif
if get_option('char-encoding') == 'utf8'
  conf.set('CHAR_ENCODING_UTF8', 1)
elif get_option('char-encoding') == 'locale'
  conf.set('CHAR_ENCODING_LOCALE', 1)
endif
if have_hp2ps
  conf.set('HAVE_HP2PS', 1)
endif

# Macros that are tested with #ifdef as well as #if, so only define the
# ones that are on
option_macros = {
  'profiling': ['PROFILING'],
  'checks': ['DEBUG_CODE', 'DEBUG_PRINTER', 'CHECK_TAGS'],
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
  subdir('tests')
endif
subdir('docs')

install_subdir('demos', install_dir: cupiddir)
install_data('README.md', 'License', 'Credits', install_dir: cupiddir)

summary({
  'cupiddir': cupiddir,
  'readline': use_readline,
  'character encoding': get_option('char-encoding'),
  'Haskell libraries': get_option('libraries'),
  'fficupid compile command': mkdll_cmd,
}, section: 'Configuration')
