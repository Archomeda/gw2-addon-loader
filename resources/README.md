# Fonts
Guild Wars 2 Addon Loader uses various fonts.
Its main font is the same as Guild Wars 2 itself, a slightly modified version of Trebuchet MS.
It also uses the icon fonts [Material Icons](https://material.io/tools/icons/) and [Octicons](https://octicons.github.com/).

The fonts are precompiled into a special type of bitmap font file.
This makes it possible to have consistent fonts across all computers.
In order to compile the font file, do the following:
1. Download [Bitmap Font Generator](http://www.angelcode.com/products/bmfont/)
2. Load the fonts into aforementioned application and use the following settings (anything else that isn't explicitly mentioned, should be disabled):
  - Size: Whatever is necessary
  - Font smoothing: Enabled
  - ClearType: Enabled
  - Outline thickness: 1
  - Export padding: 0
  - Export spacing: 1
  - Export width x height: 512 x 512
  - Export bit depth: 32
  - Export channels: A outline, R glyph, G glyph, B glyph
  - Export file format: Binary
  - Export textures: PNG
  - Export compression: Deflate
3. Export it
4. Compile Guild Wars 2 Addon Loader tools
5. Run the font-atlas-packer tool (can be found in the Debug/tools folder after compilation), use the FNT files as command line arguments, and place `-m` before any (except the first) file to merge it with the previous one (check the source file for more information)
6. If no errors have occured, the ATL file should be placed next to the first input file
