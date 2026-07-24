#!/usr/bin/env ruby
#
# create built-in symbol table in ROM
#
#  Copyright (C) 2015-      Kyushu Institute of Technology.
#  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
#  Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#
# (usage)
# ruby make_symbol_table.rb [option]
#
#  -o output filename.
#  -i input filename.
#  -a Targets all .c files in the current directory.
#  -v verbose
#  --path-c /path/to/*.c
#  --path-rb /path/to/*.rb
#

require "optparse"
require "ripper"
require_relative "common_sub"

APPEND_SYMBOL = [
  "+", "-", "*", "/", "==", "<", "<=", ">", ">=", "initialize", "PI", "E", "method_missing",
]


##
# verbose print
#
def vp( s, level = 1 )
  STDERR.puts s  if $opts[:v] >= level
end


##
# parse command line option
#
def get_options
  opt = OptionParser.new
  ret = {:in_files=>[], :path_c=>[], :path_rb=>[], :v=>0}

  opt.on("-i input file(s)") {|v| ret[:in_files] << v }
  opt.on("-o output file") {|v| ret[:out_file] = v }
  opt.on("-a", "--all", "targets all .c files") {|v| ret[:all] = v }
  opt.on("-v", "--verbose", "verbose mode") {|v| ret[:v] += 1 }
  opt.on("--path-c path", "path to target .c files") {|v| ret[:path_c] << v}
  opt.on("--path-rb path", "path to target .rb files") {|v| ret[:path_rb] << v}
  opt.parse!(ARGV)
  return ret

rescue OptionParser::MissingArgument =>ex
  STDERR.puts ex.message
  return nil
end


##
# find source files
#
def find_sources( opts, mode )
  case mode
  when :c
    ext = ".c"
    glob_ext = "*.c"
    opt_path = :path_c
  when :rb
    ext = ".rb"
    glob_ext = "*.rb"
    opt_path = :path_rb
  else
    raise "mode error."
  end

  ret = opts[:in_files].select {|filename|
    File.extname(filename) == ext
  }

  ret.concat( Dir.glob( glob_ext))  if opts[:all]
  opts[opt_path].each {|path|
    ret.concat( Dir.glob( File.join( path, glob_ext )))
  }

  return ret
end


##
# read *.c file and extract symbols.
#
def fetch_builtin_symbol_c( filename )
  ret = []
  vp("Process '#{filename}'")

  File.open( filename ) {|file|
    while src = get_method_table_source( file )
      param = parse_source_string( src )
      exit 1 if !param

      param[:classes].each {|cls|
        if cls[:class]
          vp("Found class #{cls[:class]}, #{cls[:methods].to_a.size } methods.")
        elsif cls[:module]
          vp("Found module #{cls[:module]}, #{cls[:methods].to_a.size } methods.")
        else
          raise "Not fund CLASS or MODULE declare"
        end

        cnest = []
        (cls[:class] || cls[:module]).split("::").each {|cls|
          ret << cls
          ret << (cnest << cls).join("::")
        }

        cls[:methods]&.each {|m| ret << m[:name] }
      }
    end
  }
  ret.uniq!

  return ret
end


##
# read *.rb file and extract symbols.
#
def fetch_builtin_symbol_rb( filename )
  ret = []
  vp("Process '#{filename}'")

  File.open( filename ) {|file|
    s_exp = Ripper.sexp( File.read(file))
    _parse_rb_sexp( s_exp[1], ret )
  }

  return ret
end


##
# (sub) parse ruby s-exp
#
def _parse_rb_sexp( s_exp, res )
  s_exp.each {|s_exp1|
    case s_exp1[0]
    when :def, :alias
      s_exp2 = s_exp1.flatten
      idx = s_exp2.find_index(:@ident)
      if idx
        vp("Found method #{s_exp2[idx+1]}")
        res << s_exp2[idx+1]
      end
      s_exp2.each_with_index {|item,idx|
        if item == :@label
          vp("Found label #{s_exp2[idx+1]}")
          res << s_exp2[idx+1].chop
        end
      }

    when :class, :module
      s_exp1.each {|s_exp2|
        next if !s_exp2.is_a?(Array)

        if s_exp2[0] == :const_ref && s_exp2[1][0] == :@const
          vp("Found #{s_exp1[0]} #{s_exp2[1][1]}")
          res << s_exp2[1][1]
        elsif s_exp2[0] == :bodystmt
          _parse_rb_sexp( s_exp2[1], res )
        end
      }

    when :assign
      if s_exp1[1][0] == :var_field && s_exp1[1][1][0] == :@const
        vp("Found constant #{s_exp1[1][1][1]}")
        res << s_exp1[1][1][1]
      end
    end
  }
end


##
# convert nested symbol to internal code
#
def convert_nested_symbol_to_internal_code( symbol_table, nested_symbol )
  # separate nested symbol
  # (e.g.) "C1::C2::C3" to "C1::C2","C3"
  a = nested_symbol.split("::")
  child_symbol = a.pop
  parent_symbol = a.join("::")

  # convert recursive if parent symbol is nested
  if parent_symbol.include?("::")
    s = convert_nested_symbol_to_internal_code( symbol_table, parent_symbol )
    idx = symbol_table.index(s)
    raise if !idx
  else
    idx = symbol_table.index(parent_symbol)
    raise if !idx
  end

  # convert internal code
  s = '\\x7f""'
  3.downto(0) {|i| s << (((idx >> (i*4)) & 0x0f) + 0x30).chr }
  idx = symbol_table.index(child_symbol)
  raise if !idx
  3.downto(0) {|i| s << (((idx >> (i*4)) & 0x0f) + 0x30).chr }

  return s
end


##
# convert nested symbols
#
def convert_nested_symbols( all_symbols )
  normal_symbols = [nil]
  nested_symbols = []

  all_symbols.sort.uniq.each {|symbol|
    if symbol.include?("::")
      nested_symbols << symbol
    else
      normal_symbols << symbol
    end
  }

  # Sort by nesting level
  nested_symbols = nested_symbols.sort_by {|symbol|
    [symbol.split("::").size, symbol]
  }

  # convert internal code
  symbol_table = normal_symbols.dup
  nested_symbols.each {|symbol|
    symbol_table << convert_nested_symbol_to_internal_code(symbol_table, symbol)
  }

  # update to ["Nested::Symbol", "InternalCode"]
  nested_symbols.each_with_index {|symbol,i|
    i += normal_symbols.size
    symbol_table[i] = [symbol, symbol_table[i]]
  }

  return symbol_table
end


##
# write symbol table file.
#
def write_file( symbol_table )
  vp("Output file '#{$opts[:out_file] || "STDOUT"}'")
  begin
    file = $opts[:out_file] ? File.open( $opts[:out_file], "w" ) : $stdout
  rescue Errno::ENOENT
    puts "File can't open. #{output_filename}"
    exit 1
  end

  # create an array of symbol strings.
  symstr = ""
  symbol_table.each_with_index {|v,i|
    if i == 0
      symstr << "  0,\t\t\t// (ERROR or RESERVED)\n"
      next
    end

    v1,v2 = v.is_a?(Array) ? v : [v,v]
    s1 = %!  "#{v2}",!
    symstr << s1 << "\t" * [3 - s1.size / 8, 1].max
    symstr << "// MRBC_SYMID_#{rename_for_symbol(v1)} = #{i}(0x#{i.to_s(16)})\n"
  }

  # create enum
  enumstr = ""
  symbol_table.each_with_index {|v,i|
    next if i == 0

    v1 = v.is_a?(Array) ? v[0] : v
    enumstr << "  MRBC_SYMID_#{rename_for_symbol(v1)} = #{i},\n"
  }

  file.puts <<EOL
/* Auto generated by make_symbol_table.rb */
#ifndef MRBC_SRC_AUTOGEN_BUILTIN_SYMBOL_H_
#define MRBC_SRC_AUTOGEN_BUILTIN_SYMBOL_H_

#define MRBC_BUILTIN_SYMBOL_MAX #{symbol_table.size-1}

#if defined(MRBC_DEFINE_SYMBOL_TABLE)
static const char *builtin_symbols[] = {
#{symstr}};
#endif

enum {
#{enumstr}};

#endif
EOL

  file.close  if $opts[:out_file]
end


##
# debug display symbol table array
#
def display_symbol_table( symbol_table )
  symbol_table.each_with_index {|v, i|
    printf("%04x: %s\n", i, v.inspect)
  }
end


##
# main
#
$opts = get_options()
exit if !$opts

source_files_c = find_sources( $opts, :c )
source_files_rb = find_sources( $opts, :rb )
if source_files_c.empty? && source_files_rb.empty?
  STDERR.puts "File not given."
  exit 1
end

all_symbols = []
source_files_c.each {|filename|
  all_symbols.concat( fetch_builtin_symbol_c( filename ) )
}
source_files_rb.each {|filename|
  all_symbols.concat( fetch_builtin_symbol_rb( filename ) )
}
all_symbols.concat( APPEND_SYMBOL )

symbol_table = convert_nested_symbols( all_symbols )
vp("Total number of built-in symbols: #{symbol_table.size}")
#display_symbol_table( all_symbols ); puts
#display_symbol_table( symbol_table )

write_file( symbol_table )

vp("Done")
