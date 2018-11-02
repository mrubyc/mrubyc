MRuby::Gem::Specification.new('mrubyc') do |spec|
  spec.license = 'BSD3 Clause'
  spec.author = 'mruby/c developers'
  spec.summary = 'more compact implementation of mruby VM'

  cc.defines << 'MRBC_DEBUG' if build.debug_enabled?
  cc.include_paths << "#{dir}/core"
  cc.flags << '-Wno-declaration-after-statement'

  mrbc.compile_options << ' -E' # force big endian

  lib = libfile "#{build.build_dir}/lib/libmrubyc"
  ext_lib = libfile "#{build.build_dir}/lib/libmrubyc_ext"

  objs = Dir.glob("#{dir}/core/*.c").map{|f| objfile(f.pathmap("#{build_dir}/core/%n")) }
  objs += Dir.glob("#{dir}/core/hal_posix/*.c").map{|f| objfile(f.pathmap("#{build_dir}/core/hal_posix/%n")) }

  mrblib_srcs = Dir.glob("#{dir}/stdlib/*.rb")
  mrblib_gen = "#{build_dir}/mrblib.c"
  objs << objfile(mrblib_gen)

  file objfile(mrblib_gen) => mrblib_gen do |t|
    cc.run t.name, t.prerequisites.first, []
  end

  file mrblib_gen => mrblib_srcs + [build.mrbcfile, __FILE__] do |t|
    File.open(t.name, 'w') do |f|
      mrbc.run f, mrblib_srcs, 'mrblib_bytecode'
    end
  end

  file lib => objs do |t|
    archiver.run t.name, t.prerequisites
  end

  ext_objs = Dir.glob("#{dir}/ext/*.c").map{|f| objfile(f.pathmap("#{build_dir}/ext/%n")) }
  file ext_lib => ext_objs do |t|
    archiver.run t.name, t.prerequisites
  end

  {
    'main.c' => 'mrubyc',
    'main_sample.c' => 'mrubyc_sample',
    'main_concurrent.c' => 'mrubyc_concurrent',
    'main_myclass.c' => 'mrubyc_myclass',
  }.each do |bin_src, bin|
    src = "#{dir}/sample_c/#{bin_src}"
    obj = objfile src.pathmap("#{build_dir}/sample_c/%n")

    file obj => src do |t|
      cc.run t.name, t.prerequisites.first, [], ["#{dir}/ext"]
    end

    file "#{build.build_dir}/bin/#{bin}" => [obj, lib, ext_lib] do |t|
      build.linker.run t.name, t.prerequisites
    end

    build.bins << bin
  end

  file "#{dir}/sample_c/sample01.c" => ["#{dir}/sample_c/myclass.rb", build.mrbcfile, __FILE__] do |t|
    File.open(t.name, 'w') do |f|
      mrbc.run f, ["#{dir}/sample_c/myclass.rb"], 'ary'
    end
  end
  file objfile("#{dir}/sample_c/main_sample.c".pathmap("#{build_dir}/sample_c/%n")) => "#{dir}/sample_c/sample01.c"
end
