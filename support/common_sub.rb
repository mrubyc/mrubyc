#!/usr/bin/env ruby
#
# common sub function.
#
#  Copyright (C) 2015- Kyushu Institute of Technology.
#  Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#
#

RENAME_CHAR = {
  "!"=>"NOT",   # or "E"
  "%"=>"MOD",
  "&"=>"AND",
  "*"=>"MUL",
  "+"=>"PLUS",
  "-"=>"MINUS",
  "/"=>"DIV",
  "<"=>"LT",
  "="=>"EQ",
  ">"=>"GT",
  "?"=>"Q",
  "@"=>"AT",
  "["=>"BL",
  "]"=>"BR",
  "^"=>"XOR",
  "|"=>"OR",
  "~"=>"NEG"
}

##
# rename for symbol
#
def rename_for_symbol(s)
  return "#{$1}_E"  if /^(.+)!$/ =~ s

  r = ""
  s.each_char {|ch|
    if !RENAME_CHAR[ch]
      r << ch
    else
      r << "_"  if !r.empty?
      r << RENAME_CHAR[ch]
    end
  }
  return r
end


##
# strip double quot
#
def strip_double_quot( s )
  ret = s.dup
  ret.slice!(0)  if ret[0] == '"'
  ret.chop! if ret[-1] == '"'

  return ret
end


#
# find and get source code.
#
def get_method_table_source( file )
  ret = nil

  # find magic comment
  while txt = file.gets
    if /\/\*\s+MRBC_AUTOGEN_METHOD_TABLE/ =~ txt
      ret = ""
      break
    end
  end
  return nil  if !ret

  while txt = file.gets
    # skip comment and empty line.
    txt = txt.split("//", 2)[0].strip
    next if txt.empty?

    break  if txt.start_with?("*/")
    ret << txt << "\n"
  end

  return ret
end


##
# parser
#
# (note)
#  create hash, below.
# {
#   file: "c_range_method_table.h",
#   classes: [ {
#     class: "Range",
#     super: "Object",
#     methods: [
#       { name:"first", func:"c_range_first", if_exp:[...] },... ]
#   } ]
# }
#
# If you don't specify the superclass, the default value will be added.
#
def parse_source_string( src )
  flag_error = false
  ret = { file: nil, classes: [] }
  cls = nil
  if_exp = []

  src.each_line {|txt|
    txt.chomp!

    # case preprocessor '#if'
    if /^#\s*if/ =~ txt
      if_exp << txt
      next
    end
    if /^#\s*endif/ =~ txt
      if_exp.pop
      next
    end

    if /^([A-Z]+)\s*\((.*)\)/ !~ txt
      puts "Error: #{txt}"
      flag_error = true
      next
    end

    # case CLASS, METHOD and etc.
    key = $1
    args = $2.split(",").map {|s| s.strip }

    flag_arg_ok = true
    case key.upcase
    when "FILE", "APPEND"
      ret[key.downcase.to_sym] = strip_double_quot(args[0])

    when "CLASS"
      cls = { class: strip_double_quot(args[0]), :super=>"Object" }
      ret[:classes] << cls

    when "MODULE"
      cls = { module: strip_double_quot(args[0]), :super=>"0" }
      ret[:classes] << cls

    when "SUPER"
      if !cls
        puts "Error: need CLASS parameter first."
        next
      end
      cls[:super] = strip_double_quot(args[0])

    when "METHOD"
      if !cls
        puts "Error: need CLASS parameter first."
        next
      end
      m = { name:strip_double_quot(args[0]), func:strip_double_quot(args[1]) }
      m[:if_exp] = if_exp.dup  if !if_exp.empty?
      cls[:methods] ||= []
      cls[:methods] << m

    else
      puts "Error: Invalid keyword. '#{key}'"
      flag_error = true
    end
  }

  return flag_error ? nil : ret
end
