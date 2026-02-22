#!/usr/bin/env ruby
#
# Generate compressed Unicode case mapping tables for mruby/c
#
# Usage:
#   1. Download UnicodeData.txt from https://www.unicode.org/Public/UCD/latest/ucd/
#   2. Run: ruby generate_unicode_case_tables.rb path/to/UnicodeData.txt
#   3. Output goes to src/_autogen_unicode_case.h
#
# The script generates compressed case mapping tables using:
# - Range tables with XOR patterns for common transformations
# - Exception tables for irregular mappings
#
# Total size: ~2.5KB for full BMP (U+0000 to U+FFFF) support
#

if ARGV.empty?
  puts "Usage: #{$0} <path/to/UnicodeData.txt> [output_path]"
  puts "Download from: https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt"
  exit 1
end

unicode_data_path = ARGV[0]
output_path = ARGV[1] || File.join(File.dirname(__FILE__), '../src/_autogen_unicode_case.h')

# Parse UnicodeData.txt
upper_map = {}  # codepoint => uppercase codepoint
lower_map = {}  # codepoint => lowercase codepoint

File.readlines(unicode_data_path).each do |line|
  fields = line.strip.split(';', -1)
  cp = fields[0].to_i(16)
  next if cp > 0xFFFF  # BMP only
  next if fields.size < 14

  upper = fields[12] && !fields[12].empty? ? fields[12].to_i(16) : nil
  lower = fields[13] && !fields[13].empty? ? fields[13].to_i(16) : nil

  upper_map[cp] = upper if upper && upper != cp
  lower_map[cp] = lower if lower && lower != cp
end

puts "Total uppercase mappings: #{upper_map.size}"
puts "Total lowercase mappings: #{lower_map.size}"

# Find the most common XOR values
def find_common_xors(mapping, min_count = 20)
  xor_counts = Hash.new(0)
  mapping.each { |from, to| xor_counts[from ^ to] += 1 }
  xor_counts.select { |_, count| count >= min_count }
             .sort_by { |_, count| -count }
             .map(&:first)
end

# For each XOR value, find ranges of contiguous or near-contiguous chars
def find_ranges_for_xor(mapping, xor_value)
  chars = mapping.select { |from, to| (from ^ to) == xor_value }.keys.sort
  return [] if chars.empty?

  ranges = []
  start = chars.first
  prev = start

  chars[1..].each do |cp|
    gap = cp - prev
    # Allow gaps up to 2 for better range merging
    if gap <= 2
      prev = cp
    else
      ranges << [start, prev] if prev >= start
      start = cp
      prev = cp
    end
  end
  ranges << [start, prev] if prev >= start
  ranges
end

# Build optimized tables using multiple XOR patterns
def build_compressed_tables(mapping, name)
  remaining = mapping.dup

  # Common XOR patterns (sorted by frequency)
  common_xors = find_common_xors(mapping, 15)
  puts "\n#{name} - using XOR patterns: #{common_xors.map { |x| '0x%02X' % x }.join(', ')}"

  ranges = []

  common_xors.each do |xor|
    xor_ranges = find_ranges_for_xor(remaining, xor)
    xor_ranges.each do |start, stop|
      # Only use range if it covers at least 3 characters or saves space
      covered = (start..stop).count { |cp| remaining[cp] && (cp ^ remaining[cp]) == xor }
      if covered >= 2
        ranges << { xor: xor, start: start, end: stop }
        (start..stop).each { |cp| remaining.delete(cp) if remaining[cp] && (cp ^ remaining[cp]) == xor }
      end
    end
  end

  puts "  Ranges: #{ranges.size}"
  puts "  Exceptions: #{remaining.size}"

  # Calculate size: ranges are 6 bytes each (xor:2 + start:2 + end:2)
  # Exceptions are 4 bytes each (from:2 + to:2)
  range_bytes = ranges.size * 6
  exception_bytes = remaining.size * 4
  puts "  Estimated size: #{range_bytes + exception_bytes} bytes"

  { ranges: ranges, exceptions: remaining }
end

upper_data = build_compressed_tables(upper_map, "Upcase")
lower_data = build_compressed_tables(lower_map, "Downcase")

# Generate C code
c_code = <<~C
/*
 * Unicode BMP case mapping tables (auto-generated)
 * Generated from UnicodeData.txt
 *
 * Compression strategy:
 * 1. Range table with XOR patterns (for contiguous mappings)
 * 2. Exception table for irregular mappings
 *
 * To regenerate: ruby generate_case_tables.rb
 */

#if MRBC_USE_STRING_UTF8 && MRBC_USE_UNICODE_CASE

/* Range entry: XOR value, start codepoint, end codepoint (inclusive) */
typedef struct {
  uint16_t xor_val;
  uint16_t start;
  uint16_t end;
} case_range_t;

/* Exception entry: from codepoint, to codepoint */
typedef struct {
  uint16_t from;
  uint16_t to;
} case_exception_t;

C

# Generate upcase ranges
c_code += "/* Upcase ranges: #{upper_data[:ranges].size} entries */\n"
c_code += "static const case_range_t upcase_ranges[] = {\n"
upper_data[:ranges].sort_by { |r| r[:start] }.each do |r|
  c_code += "  { 0x%04X, 0x%04X, 0x%04X },\n" % [r[:xor], r[:start], r[:end]]
end
c_code += "};\n\n"

# Generate upcase exceptions
c_code += "/* Upcase exceptions: #{upper_data[:exceptions].size} entries */\n"
c_code += "static const case_exception_t upcase_exceptions[] = {\n"
upper_data[:exceptions].sort.each do |from, to|
  c_code += "  { 0x%04X, 0x%04X },\n" % [from, to]
end
c_code += "};\n\n"

# Generate downcase ranges
c_code += "/* Downcase ranges: #{lower_data[:ranges].size} entries */\n"
c_code += "static const case_range_t downcase_ranges[] = {\n"
lower_data[:ranges].sort_by { |r| r[:start] }.each do |r|
  c_code += "  { 0x%04X, 0x%04X, 0x%04X },\n" % [r[:xor], r[:start], r[:end]]
end
c_code += "};\n\n"

# Generate downcase exceptions
c_code += "/* Downcase exceptions: #{lower_data[:exceptions].size} entries */\n"
c_code += "static const case_exception_t downcase_exceptions[] = {\n"
lower_data[:exceptions].sort.each do |from, to|
  c_code += "  { 0x%04X, 0x%04X },\n" % [from, to]
end
c_code += "};\n\n"

c_code += "#define UPCASE_RANGES_COUNT #{upper_data[:ranges].size}\n"
c_code += "#define UPCASE_EXCEPTIONS_COUNT #{upper_data[:exceptions].size}\n"
c_code += "#define DOWNCASE_RANGES_COUNT #{lower_data[:ranges].size}\n"
c_code += "#define DOWNCASE_EXCEPTIONS_COUNT #{lower_data[:exceptions].size}\n\n"
c_code += "#endif /* MRBC_USE_STRING_UTF8 && MRBC_USE_UNICODE_CASE */\n"

File.write(output_path, c_code)

total_ranges = upper_data[:ranges].size + lower_data[:ranges].size
total_exceptions = upper_data[:exceptions].size + lower_data[:exceptions].size
total_bytes = total_ranges * 6 + total_exceptions * 4

puts "\n" + "="*60
puts "Total table size: #{total_bytes} bytes (#{(total_bytes / 1024.0).round(2)} KB)"
puts "  Ranges: #{total_ranges} × 6 = #{total_ranges * 6} bytes"
puts "  Exceptions: #{total_exceptions} × 4 = #{total_exceptions * 4} bytes"
puts "Written to #{output_path}"
