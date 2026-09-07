# Regression tests: each NAME.hs under this directory, run by run-tests and
# compared with NAME.stdout.  See the comment at the top of run-tests for
# the other files a test may have.  Run them with `meson test -C <build>`;
# a single one with `tests/run-tests <cuddles> <runcuddles> '<packages>/*' tests NAME`.

run_tests = find_program('run-tests')

test('regression', run_tests,
  args: [cuddles, runcuddles, cuddlesdir_build / 'packages' / '*',
         meson.current_source_dir()],
  depends: [cuddles, runcuddles, libraries_stamp],
  timeout: 600, verbose: true)
