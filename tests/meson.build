# Regression tests: each NAME.hs under this directory, run by run-tests and
# compared with NAME.stdout.  See the comment at the top of run-tests for
# the other files a test may have.  Run them with `meson test -C <build>`;
# a single one with `tests/run-tests <cupid> '<packages>/*' tests NAME`.

run_tests = find_program('run-tests')

test('regression', run_tests,
  args: [cupid, cupiddir_build / 'packages' / '*',
         meson.current_source_dir()],
  depends: [cupid, libraries_stamp],
  timeout: 600, verbose: true)

# gdb reads function arguments for the tokens and cells, which an optimised
# build may not have kept; see check-doc-sessions.
doc_session_flags = get_option('optimization') == '0' ? [] : ['--no-gdb']

test('doc-sessions', find_program('check-doc-sessions'),
  args: doc_session_flags + [cupid, cupiddir_build / 'packages' / '*',
                             meson.project_source_root()],
  depends: [cupid, libraries_stamp],
  timeout: 300, verbose: true)
