require 'yaml'

MRuby::Gem::Specification.new('mrubyc') do |spec|
  spec.license = 'BSD3 Clause'
  spec.author = 'mruby/c developers'
  spec.summary = 'more compact implementation of mruby VM'

  cc.defines << 'MRBC_DEBUG' if build.debug_enabled?
  cc.include_paths << "#{dir}/core" << "#{build_dir}/include"
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

  table_gen = "#{build_dir}/table.c"
  file table_gen => ["#{dir}/method_table.yml", __FILE__] do |t|
    FileUtils.mkdir_p File.dirname t.name
    table = YAML.load File.read "#{dir}/method_table.yml"
    File.open(t.name, 'w') do |f|
      f.write %[#include <stdlib.h>\n]
      f.write %[#include "class.h"\n]
      f.write %[#include "vm.h"\n]
      f.write %[#include "static.h"\n]

      fwds = []
      defs = []

      defs << %[void mrbc_init_method_table(mrbc_vm *vm) {\n]

      table.each do |cls, methods|
        caml_cls = cls.gsub(/[A-Z]/){|v| "_#{v.downcase}"}.sub(/\A_/, '')
        caml_cls = 'vm' if cls == 'VM'
        super_cls = :mrbc_class_object
        super_cls = :NULL unless methods['_super']
        cls_sym = "mrbc_class_#{caml_cls}".sub(/_class\z/, '')
        defs << %[#{cls_sym} = mrbc_define_class(vm, "#{cls}", #{super_cls});\n]

        meth_writer = proc do |c_sym, meth, full_sym|
          fwds << full_sym
          if meth.kind_of? String
            defs << %[mrbc_define_method(vm, #{cls_sym}, "#{meth}", #{full_sym});]
          elsif meth.kind_of? TrueClass
            defs << %[mrbc_define_method(vm, #{cls_sym}, "#{c_sym}", #{full_sym});]
          elsif meth['methods']
            meth['methods'].each do |meth_name|
              defs << %[mrbc_define_method(vm, #{cls_sym}, "#{meth_name}", #{full_sym});]
            end
          elsif meth['ineffect']
            defs << %[mrbc_define_method(vm, #{cls_sym}, "#{c_sym}", c_ineffect);]
          elsif meth['debug']
            defs << %[#ifdef MRBC_DEBUG]
            defs << %[mrbc_define_method(vm, #{cls_sym}, "#{c_sym}", #{full_sym});]
            defs << %[#endif]
          else
            raise "uknown method specifier: #{meth}"
          end
        end

        defs << %[#if MRBC_USE_#{methods['_feature'].upcase}] if methods['_feature']

        methods.each do |c_sym, meth|
          if c_sym == '_features'
            meth.each do |feat, feat_methods|
              defs << %[#if MRBC_USE_#{feat.upcase}]
              feat_methods.each do |feat_c_sym, feat_meth|
                meth_writer.call c_sym, feat_meth, :"mrbc_methods_#{cls}_#{feat_c_sym}"
              end
              defs << %[#endif]
            end
            next
          end

          next if c_sym.start_with? '_'
          meth_writer.call c_sym, meth, :"mrbc_methods_#{cls}_#{c_sym}"
        end

        defs << %[#endif // MRBC_USE_#{methods['_feature'].upcase}] if methods['_feature']
      end

      defs << %[}\n]

      f.write fwds.map{|v| "extern void #{v}(mrbc_vm *vm, mrbc_value v[], int argc);" }.join("\n") + "\n"

      f.write defs.join("\n") + "\n"

    end
  end
  file objfile(table_gen) => table_gen do |t|
    cc.run t.name, t.prerequisites.first, []
  end
  objs << objfile(table_gen)

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

  file "#{build_dir}/include/sample01.c" => ["#{dir}/sample_c/sample.rb", build.mrbcfile, __FILE__] do |t|
    FileUtils.mkdir_p File.dirname t.name
    File.open(t.name, 'w') do |f|
      mrbc.run f, ["#{dir}/sample_c/sample.rb"], 'ary'
    end
  end
  file objfile("#{dir}/sample_c/main_sample.c".pathmap("#{build_dir}/sample_c/%n")) => "#{build_dir}/include/sample01.c"
end
