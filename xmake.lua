set_project( "SfgGenerator" )

set_version( "0.0.1", { build = "%Y%m%d", soname = true } )

set_warnings( "allextra" )

add_rules( "mode.debug", "mode.release", "mode.releasedbg", "mode.minsizerel" )

set_languages( "c++20" )

if is_plat( "windows" ) then
  add_cxflags( "/Zc:__cplusplus" )
  add_cxflags( "/Zc:preprocessor" )

  add_cxflags( "/permissive-" )
end

-- Set MSVC runtime
if is_plat( "windows" ) and is_mode( "release" ) then
  set_runtimes( "MD" ) -- MultiThreadedDLL (MD or MDd)
end
-- Set macOS deployment target
if is_plat( "macosx" ) then
  add_cxxflags( "-mmacosx-version-min=12.0" )
end
-- Common compiler flags
if is_plat( "macosx" ) or is_plat( "linux" ) then
  add_cxxflags( "-fvisibility=hidden", "-fvisibility-inlines-hidden", "-Werror=return-type",
                "-Werror=unused-result", "-Wno-unused-parameter", "-Wuninitialized", "-Wvla",
                "-Wmultichar", "-Werror=non-virtual-dtor", "-Woverloaded-virtual" )

  if is_plat( "macosx" ) then
    add_cxxflags( "-Wmost", "-Wsuper-class-method-mismatch" )
  end
end

if is_plat( "windows" ) then
  add_cxxflags( "/utf-8", "/Zi", "/FS" )
  add_links( "Ole32" )
  add_links( "Shell32" )
  if is_mode( "release" ) then
    add_ldflags( "/OPT:REF", "/OPT:ICF=3", "/Qvec", "/GR", { force = true } )
    --add_ldflags( "/OPT:ICF=3", "/Qvec", "/GR", { force = true } )
    --add_ldflags( "/Qvec", "/GR", { force = true } )
    --add_ldflags( "/GR", { force = true } )
  end
end

add_requireconfs( "*", "**", "*.**", "**.*", "**.**", { system = false } )
--add_requireconfs( "*", "**", "*.**", "**.*", "**.**", { configs = { shared = get_config( "kind" ) == "shared" } } )
add_requireconfs( "*", { configs = { shared = get_config( "kind" ) == "shared" } } )

add_requires( "spdlog" )
add_requires( "fmt" )
add_requires( "hedley" )
add_requires( "protoc", "protobuf-cpp" )
add_requires( "qt5core", "qt5gui", "qt5widgets" )
add_requires( "vcpkg::clap-cleveraudio", { alias = "vcpkg-clap" } )

add_requireconfs( "spdlog", { configs = { header_only = true, fmt_external_ho = true } } )
add_requireconfs( "fmt", { configs = { header_only = true, unicode = true } } )
add_requireconfs("**.abseil", { override = true, system = false } ) -- https://github.com/xmake-io/xmake-repo/issues/9228#issuecomment-3828155467

target( "SfgGeneratorMain" )
  add_rules( "qt.shared", { public = false } )
  set_encodings( "utf-8" )

  set_default( false )
  set_group( "LIBS" )

  add_packages( "spdlog", { public = false } )
  add_packages( "fmt", { public = false } )
  add_packages( "hedley", { public = false } )
  add_packages( "protoc", "protobuf-cpp", { public = false } )
  add_packages( "qt5core", "qt5gui", "qt5widgets", { public = false } )
  add_packages( "vcpkg-clap", { public = false } )

  add_defines( "SFG_GEN_EXPORT_CLAP_INIT", { public = false } )

  add_rules( "protobuf.cpp" )
  add_files( "proto/**.proto", { proto_public = false, proto_rootdir = path.join( "proto" ) } )

  add_includedirs( "include", { public = true } )
  add_headerfiles( "include/(plugin/main.hpp)" )
  add_headerfiles( "include/common/*.hpp" )
  add_headerfiles( "include/plugin/audio_lerp_efffect.hpp" )
  add_headerfiles( "include/plugin/base_plugin.hpp" )
  add_headerfiles( "include/plugin/noise_generator.hpp" )
  add_headerfiles( "include/plugin/param_multiplex.hpp" )
  add_headerfiles( "include/ui/*.hpp" )
  add_files( "src/common/*.cpp" )
  add_files( "src/plugin/*.cpp" )
  add_files( "include/ui/*.hpp" )
  add_files( "src/ui/*.cpp" )

  add_frameworks( "QtCore", "QtGui", "QtWidgets", { public = false } )
target_end()

target( "SfgGenerator" )
  set_kind( "shared" )
  set_encodings( "utf-8" )

  set_default( true )
  set_group( "LIBS" )

  add_packages( "vcpkg-clap", { public = false } )

  add_files( "src/main.cpp" )

  if is_plat( "linux" ) then
    add_ldflags( "-Wl,--version-script=" .. path.join( os.scriptdir(), "linux-clap-plugins.version" ), "-Wl,-z,defs", { force = true } )
    set_filename( "SfgGenerator.clap" )
  elseif is_plat( "macosx" ) then
    add_ldflags( "-exported_symbols_list " .. path.join( os.scriptdir(), "macos-symbols.txt" ), { force = true } )
    set_values( "bundle.extension", "clap" )
    set_values( "bundle.identifier", "de.sfgrena.SfgGenerator" )
    set_values( "bundle.version", "1" )
    set_values( "bundle.shortver", "1" )
    set_values( "bundle.plist", path.join( os.scriptdir(), "plugins.plist.in" ) )
  elseif is_plat( "windows" ) then
    set_filename( "SfgGenerator.clap" )
  end

  add_rules( "utils.symbols.export_list", { symbols = { "clap_entry" } } )

  on_test( function ( target, opt )
    local args = {
      "--verbosity", "quiet",
      "validate",
      "--hide-output", "--only-failed",
      --"--in-process",  -- to test
      --"--test-filter", "state-reproducibility-basic",  -- to test
      path.join( target:scriptdir(), target:targetdir(), "SfgGenerator.clap" )
    }
    local out_file = os.tmpfile()
    local err_file = os.tmpfile()
    os.vrunv( "C:\\_Programs\\clap-validator\\clap-validator", args, { stdout = out_file, stderr = err_file } )
    local cmd_std_out = io.readfile(out_file)
    local cmd_std_err = io.readfile(err_file)
    local return_val = string.match( cmd_std_out, opt.pass_outputs ) and true or false
    if return_val then
      os.rm(out_file)
      os.rm(err_file)
    else
      print( "stdout file at:", out_file )
      print( "stderr file at:", err_file )
    end
    return return_val
  end )
  add_tests( "clap-validator", { pass_outputs = ", 0 failed, " } )
target_end()

target( "clap-validator" )
  set_kind( "phony" )
  set_default( false )
  set_group( "EXES" )
  on_run( function ( target )
    import( "core.base.option" )
    import( "devel.debugger" )

    if option.get( "debug" ) then
      debugger.run( "C:\\_Programs\\clap-validator\\clap-validator", { "validate", "--in-process", "C:\\VstPlugins\\hard_clapx64\\SfgGenerator.clap" } )
    else
      os.execv( "C:\\_Programs\\clap-validator\\clap-validator", { "validate", "C:\\VstPlugins\\hard_clapx64\\SfgGenerator.clap" } )
    end
  end )
target_end()
