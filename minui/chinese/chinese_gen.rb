#!/usr/bin/ruby

require 'rubygems'
require 'iconv'
require 'RMagick'
include Magick

# Get font settings from args
font_width = ARGV[0]
font_height = ARGV[1]
font_point_size = ARGV[1]

if nil == font_width
	puts "please special font_width!"
	exit
end

if nil == font_height
	puts "please special font_height!"
	exit
end

if nil == font_point_size
	puts "please special font_point_size!"
	exit
end

puts "font_width : " + font_width
puts "font_height : " + font_height
puts "font_point_size : " + font_point_size

s = ''
# basic printable
for i in 32...128
    s = "%s%c" % [ s, i ]
end

# generate all gb2312 character
#for i in 0xa1..0xfe
#    for j in 0xa1..0xfe
#        c += "%c%c" % [ i, j ]
#    end
#end
#c = Iconv.iconv('utf-8', 'gb2312', c)[0]
#c = Iconv.iconv('utf-8', 'gb18030', c)[0]
#s += c
# gen needed character using gperf
p "preparing data"
c = ''
count = s.length
f = File.open('chinese_custom.gperf', 'w')
f.write("struct utf8_to_custom { char* name; int value; };\n")
f.write("%%\n")
# chinese punct
for j in 0xa1..0xfe
    ch = "%c%c" % [ 0xa1, j ]
    c += ch
    ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
    line = "\"%s\", %d\n" % [ ch, count ]
    f.write(line)
    count += 1
end
# full width ascii
for j in 0xa1..0xfe
    ch = "%c%c" % [ 0xa3, j ]
    c += ch
    ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
    line = "\"%s\", %d\n" % [ ch, count ]
    f.write(line)
    count += 1
end
# hiragana
for j in 0xa1..0xf3
    ch = "%c%c" % [ 0xa4, j ]
    c += ch
    ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
    line = "\"%s\", %d\n" % [ ch, count ]
    f.write(line)
    count += 1
end
# katakana
for j in 0xa1..0xf6
    ch = "%c%c" % [ 0xa5, j ]
    c += ch
    ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
    line = "\"%s\", %d\n" % [ ch, count ]
    f.write(line)
    count += 1
end
# common simplified chinese charaters
for i in 0xb0..0xd6
    for j in 0xa1..0xfe
        ch = "%c%c" % [ i, j ]
        c += ch
        ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
        line = "\"%s\", %d\n" % [ ch, count ]
        f.write(line)
        count += 1
    end
end
for j in 0xa1..0xf9
    ch = "%c%c" % [ 0xd7, j ]
    c += ch
    ch = Iconv.iconv('utf-8', 'gb2312', ch)[0]
    line = "\"%s\", %d\n" % [ ch, count ]
    f.write(line)
    count += 1
end
f.write('%%')
f.close
p = IO.popen('gperf -t chinese_custom.gperf > chinese_custom.h')
p.read
p.close
# gen a **huge** bitmap
count -= s.length
char_width = font_width.to_i
char_height = font_height.to_i
asc_width = char_width * s.length
cjk_width = count * 2 * char_width
img_width = asc_width + cjk_width
img_height = char_height
canvas = Image.new(img_width, img_height)
# so, bitmap font is better...
text = Draw.new
text.gravity = WestGravity
text.pointsize = font_point_size.to_i
text.font = 'fonts/simhei.ttf'
p "painting basic ascii"
for i in 0...96
    ch = s[i, 1]
    # what a fuck
    ch = "\\\\" if ch == "\\"
    text.text(i * char_width, 0, ch)
end
text.draw(canvas)
text.font = 'fonts/simsun.ttc'
p "painting extra characters"
for i in 0...count
    ch = Iconv.iconv('utf-8', 'gb2312', c[i * 2, 2])[0]
    text.text(i * char_width * 2 + asc_width, 0, ch)
end
text.draw(canvas)
p "writing preview file"
canvas.write('chinese.bmp')
pixels = []
f = File.open('chinese.preview.txt', 'w')
canvas.each_pixel { |p, x, y|
    r = (p.red < 0xC000 && p.green < 0xC000 && p.blue < 0xC000)
    pixels.push(r ? 1 : 0)
    o = r ? '*' : '#'
    f.write(o)
    f.write("\n") if x == img_width - 1
}
f.close
p "generating header files"
# encode pixels
out = []
n = 0
k = 0
run_val = (pixels[0] == 0) ? 0 : pixels[0];
run_count = 1
for i in 1...(img_width * img_height)
    val = (pixels[i] == 0) ? 0 : pixels[i]
    if (val == run_val) && (run_count < 127)
        run_count += 1
    else
        k += run_count
        out.push(run_count | (run_val != 0 ? 0x80 : 0x00))
        run_val = val
        run_count = 1
    end
end
k += run_count
out.push(run_count | (run_val != 0 ? 0x80 : 0x00))
out.push(0)
# gen font data
f = File.open('font_' + font_width + 'x' + font_height + '.h', 'w')
f.write("struct {\n")
f.write("\tunsigned width;\n")
f.write("\tunsigned height;\n")
f.write("\tunsigned cwidth;\n")
f.write("\tunsigned cheight;\n")
f.write("\tunsigned char rundata[];\n")
f.write("} font = {\n")
f.write("\t.width = #{img_width},\n")
f.write("\t.height = #{img_height},\n")
f.write("\t.cwidth = #{char_width},\n")
f.write("\t.cheight = #{char_height},\n")
f.write("\t.rundata = {\n")
n = 0
out.each { |b|
    f.write(("0x%02x, " % [ b ]))
    n += 1
    f.write("\n") if n  % 16 == 0
}
f.write("\t}\n")
f.write('};')
f.close

