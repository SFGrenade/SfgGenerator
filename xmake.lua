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

--add_requireconfs( "*", "**", "*.**", "**.*", "**.**", { system = false } )
--add_requireconfs( "*", "**", "*.**", "**.*", "**.**", { configs = { shared = get_config( "kind" ) == "shared" } } )
add_requireconfs( "*", { configs = { shared = get_config( "kind" ) == "shared" } } )

add_requires( "vcpkg::clap-cleveraudio", { alias = "clap" } )
add_requires( "protoc", "protobuf-cpp" )
add_requires( "spdlog" )

add_requireconfs( "spdlog", { configs = { header_only = true, std_format = false, fmt_external = false, fmt_external_ho = true, noexcept = false } } )

target( "SfgGenerator" )
  set_kind( "shared" )
  set_encodings( "utf-8" )

  set_default( true )
  set_group( "LIBS" )

  add_packages( "clap", { public = true } )
  add_packages( "protoc", "protobuf-cpp", { public = true } )
  add_packages( "spdlog", { public = true } )

  add_rules( "protobuf.cpp" )
  add_files( "proto/**.proto", { proto_public = false, proto_rootdir = path.join( "proto" ) } )

  add_includedirs( "include", { public = true } )

  add_headerfiles( "include/(*.hpp)" )
  add_files( "src/*.cpp" )

  if is_plat( "linux" ) then
    add_ldflags( "-Wl,--version-script=" .. path.join( os.scriptdir(), "linux-SfgGenerator.version" ), "-Wl,-z,defs", { force = true } )
    set_filename( "SfgGenerator.clap" )
  elseif is_plat( "macosx" ) then
    add_ldflags( "-exported_symbols_list " .. path.join( os.scriptdir(), "macos-symbols.txt" ), { force = true } )
    set_values( "bundle.extension", "clap" )
    set_values( "bundle.identifier", "org.clap.example-plugins" )
    set_values( "bundle.version", "1" )
    set_values( "bundle.shortver", "1" )
    set_values( "bundle.plist", path.join( os.scriptdir(), "plugins.plist.in" ) )
  elseif is_plat( "windows" ) then
    set_filename( "SfgGenerator.clap" )
  end

  on_install( function ( target )
    os.cp( target:targetfile(), path.join( "$(installdir)", "lib/clap" ) )
  end )

  --on_run( function ( target )
  --  print( "basename:", target:basename() )
  --  print( "filename:", target:filename() )
  --  print( "soname:", target:soname() )
  --  print( "targetfile:", target:targetfile() )
  --  print( "name:", target:name() )
  --  --print( target )
  --  local args = {
  --    "--verbosity", "quiet",
  --    "validate",
  --    "--hide-output", "--only-failed",
  --    --"--in-process",  -- to test
  --    --"--test-filter", "state-reproducibility-basic",  -- to test
  --    path.join( target:scriptdir(), target:targetfile() )
  --  }
  --  os.vrunv( "C:\\_Programs\\clap-validator\\clap-validator", args )
  --end )

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
    add_deps( "SfgGenerator" )
    on_run( function ( target )
        import( "core.base.option" )
        import( "devel.debugger" )

        local targetfile = path.absolute( target:dep( "SfgGenerator" ):targetfile() )
        if option.get( "debug" ) then
            debugger.run( "C:\\_Programs\\clap-validator\\clap-validator", { "validate", "--in-process", targetfile } )
        else
            os.execv( "C:\\_Programs\\clap-validator\\clap-validator", { "validate", targetfile } )
        end
    end )
target_end()
