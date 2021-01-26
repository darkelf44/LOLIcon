# tga2font - Converts an uncompressed TGA image of a font to a C source file. 
#
# The TGA image needs to store the character graphics of the font in a grid, with a cell size given by CHARWIDTH and
# CHARHEIGHT. The characters need to appear in their numerical order starting from 0 to CHARCOUNT, and need to be layed
# out left to right, and top to bottom. Any extra space after the last character is ignored. Any color darker then
# TRESHOLD will be part of the foreground, while the others will become the background. 

import sys, os, io

import struct

# Input/output parameters
INFILE = 'monospace.tga'
OUTFILE = '../src/monospace.c'
FONTNAME = 'monospace'

# Character set parameters
CHARCOUNT = 128
CHARWIDTH = 12
CHARHEIGHT = 18

# Conversion parameters
TRESHOLD = 0.5

# read data from file
with io.open(INFILE, 'rb') as file:
	image = file.read()

# read TGA header
(	th_idlength,
	th_cmaptype,
	th_datatype,
	th_cmaporig,
	th_cmaplength,
	th_cmapdepth,
	th_xorig,
	th_yorig,
	th_width,
	th_height,
	th_bpp,
	th_imgdesc
	) = struct.unpack('<BBBHHBHHHHBB', image[0:18])

# check header
if th_datatype == 1:
	if th_cmaptype == 0:
		raise ValueError('Missing color map!')
	if th_cmapdepth not in (8, 24, 32):
		raise ValueError('Invalid colormap depth!')
	if th_bpp != 8:
		raise ValueError('Unsupported bit depth!')
elif th_datatype == 2:
	if th_bpp not in (24, 32):
		raise ValueError('Unsupported bit depth!')
elif th_datatype == 3:
	if th_bpp != 8:
		raise ValueError('Unsupported bit depth!')
else:
	raise ValueError('Unsupported data type')

# skip image id
offset = 18 + th_idlength

# read colormap
if th_datatype == 1:
	coloroffset = offset
	colorsize = (th_cmapdepth // 8)
	offset += th_cmaplength * colorsize

# Set some flags
flip = (th_imgdesc & 32) == 0
datasize = th_bpp // 8
dataoffset = offset

# Convert the image to monochrome
offset = 0
h, w = th_height, th_width
pixels = bytearray(w * h)
for j in range(h):
	# Apply flip
	if flip:
		j = h - j - 1
	for i in range(w):
		# Read pixel color
		if th_datatype == 1:
			index = image[dataoffset + (i + j * w) * datasize]
			index = coloroffset + (index - th_cmaporig) * colorsize
			color = image[index : index + colorsize]
			bpp = th_cmapdepth
		else:
			index = dataoffset + (i + j * w) * datasize
			color = image[index : index + datasize]
			bpp = th_bpp

		# Decode pixel color
		if bpp == 8:
			r = g = b = color[0]
			a = 255
		elif bpp == 24:
			b, g, r = color
			a = 255
		elif bpp == 32:
			b, g, r, a = color
		color = ((r + g + b) * a) // (3 * 255)

		# Store pixel color
		pixels[offset] = color
		offset += 1

# Calculate table size
rows, cols = h // CHARHEIGHT, w // CHARWIDTH

# Adjust count
if CHARCOUNT > rows * cols:
	CHARCOUNT = rows * cols

# Create characters
charset = []
for n in range(CHARCOUNT):
	# Coordinates
	x, y = n % cols, n // cols
	char = []
	for j in range(CHARHEIGHT):
		line = 0
		for i in range(CHARWIDTH):
			# Get pixel
			gray = pixels[i + (x * CHARWIDTH) + (j + y * CHARHEIGHT) * w]

			# Encode pixel value
			line += int(gray < TRESHOLD * 255) << i
		char.append(line)
	charset.append(char)

# Generate output file
with open(OUTFILE, 'w') as file:
	file.write('/* GENERATED FILE - DO NOT MODIFY IT! */\n\n')
	file.write('/* Includes */\n')
	file.write('#include <stdint.h>\n\n')
	file.write('uint16_t {name}_char_count = {count};\n'.format(name=FONTNAME, count=CHARCOUNT))
	file.write('uint16_t {name}_char_width = {width};\n'.format(name=FONTNAME, width=CHARWIDTH))
	file.write('uint16_t {name}_char_height = {height};\n'.format(name=FONTNAME, height=CHARHEIGHT))
	file.write('uint16_t {name}_char_table[][{height}] = {{\n'.format(name=FONTNAME, count=CHARCOUNT, height=CHARHEIGHT))
	for char in charset:
		file.write('\t{' + ', '.join('0x{:04x}'.format(n) for n in char) + '},\n')
	file.write('};\n')
