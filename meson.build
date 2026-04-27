project('ck', 'c',
  version:          '1.0',
  default_options:  ['c_std=c11', 'warning_level=2'],
  meson_version:    '>=1.3.0',
)

cc = meson.get_compiler('c')
fs = import('fs')

add_project_arguments('-D_DEFAULT_SOURCE', language: 'c')
add_project_arguments('-D_XOPEN_SOURCE_EXTENDED', language: 'c')

if get_option('debug')
  add_project_arguments(cc.get_supported_arguments(['-ggdb3']), language: 'c')
endif

# Tone down warnings for the legacy Tk-derived sources without disabling them
# in their entirety.
add_project_arguments(cc.get_supported_arguments([
    '-Wno-unused-parameter',
    '-Wno-unused-but-set-variable',
    '-Wno-missing-field-initializers',
    '-Wno-implicit-fallthrough',
    '-Wno-deprecated-declarations',
    '-Wno-cast-function-type',
  ]),
  language: 'c',
)

conf = configuration_data()

# ckPort.h consults these via HAVE_CONFIG_H
conf.set10('HAVE_LIMITS_H',     cc.has_header('limits.h'))
conf.set10('HAVE_STDINT_H',     cc.has_header('stdint.h'))
conf.set10('HAVE_UNISTD_H',     cc.has_header('unistd.h'))
conf.set10('HAVE_SYS_SELECT_H', cc.has_header('sys/select.h'))
conf.set10('HAVE_SIGACTION',    cc.has_function('sigaction'))
conf.set10('USE_NCURSES',  true)
conf.set10('USE_NCURSESW', true)
conf.set10('CK_NC_LEAK_DEBUG', get_option('nc_leak_debug'))

deps = []
inc = [include_directories('generic')]

pkg_sources = files(
  'generic/ckBuildInfo.c',
  'generic/ckBind.c',
  'generic/ckBorder.c',
  'generic/ckButton.c',
  'generic/ckCmds.c',
  'generic/ckConfig.c',
  'generic/ckEntry.c',
  'generic/ckEvent.c',
  'generic/ckFocus.c',
  'generic/ckFrame.c',
  'generic/ckGeometry.c',
  'generic/ckGet.c',
  'generic/ckGrid.c',
  'generic/ckListbox.c',
  'generic/ckMenubutton.c',
  'generic/ckMenu.c',
  'generic/ckMessage.c',
  'generic/ckOpen.c',
  'generic/ckOption.c',
  'generic/ckPack.c',
  'generic/ckPlace.c',
  'generic/ckPreserve.c',
  'generic/ckRecorder.c',
  'generic/ckScrollbar.c',
  'generic/ckStdioSwap.c',
  'generic/ckTextBTree.c',
  'generic/ckText.c',
  'generic/ckTextDisp.c',
  'generic/ckTextIndex.c',
  'generic/ckTextMark.c',
  'generic/ckTextTag.c',
  'generic/ckTree.c',
  'generic/ckUtil.c',
  'generic/ckWindow.c',
)

subdir('teabase/init')

# Runtime Tcl library files: route through teaconf_files so they land in
# the same directory as the generated pkgIndex.tcl, both in the build dir
# (for in-place tests) and at the install location (pkg_dir).  The
# pkgIndex.tcl falls back to $dir for ::ck_library when no library/
# subdirectory exists, so a flat layout works on both sides.
teaconf_files = files(
  'library/bgerror.tcl',
  'library/button.tcl',
  'library/ckfbox.tcl',
  'library/ck.tcl',
  'library/clrpick.tcl',
  'library/comdlg.tcl',
  'library/command.tcl',
  'library/dialog.tcl',
  'library/entry.tcl',
  'library/entryx.tcl',
  'library/focus.tcl',
  'library/keylpr.tcl',
  'library/listbox.tcl',
  'library/menu.tcl',
  'library/msgbox.tcl',
  'library/optMenu.tcl',
  'library/parray.tcl',
  'library/scrollbar.tcl',
  'library/showglob.tcl',
  'library/showproc.tcl',
  'library/text.tcl',
  'library/tclIndex',
)

# Locate ncurses (wide-character variant required for Unicode support)
ncurses_dep = dependency(['ncursesw', 'ncurses'], required: true)
deps += ncurses_dep

# Generate ckUuid.h with the source revision
git = find_program('git', native: true, required: false)
ckuuid = custom_target('ckUuid.h',
  output: 'ckUuid.h',
  command: [
    build_tclsh.found() ? build_tclsh : find_program('tclsh', native: true),
    meson.current_source_dir() / 'tools/gen_ckuuid.tcl',
    meson.current_source_dir(),
    '@OUTPUT@',
  ],
  build_by_default: true,
  build_always_stale: true,
)
inc += include_directories('.')
deps += declare_dependency(sources: ckuuid)

subdir('teabase')

# Library file install path is set up via tests/test_config.tcl.in if needed
test_conf = configuration_data()
test_conf.set('CK_LIBRARY', meson.project_name())
configure_file(
  input:          'tests/test_config.tcl.in',
  output:         'test_config.tcl',
  configuration:  test_conf,
)

