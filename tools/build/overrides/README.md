# Library overrides

These files replace the matching files of the pinned upstream releases in
`../deps.json` when the Wii U build compiles them.

## libpng

`libpng/png.h` and `libpng/pngwrite.c` are altered versions of the files in
libpng 1.6.2, matching the libpng the game links. `png_image_write_to_stdio`
takes a `png_voidp` stream and caller-supplied write and flush callbacks in
place of a `FILE *`. Every other libpng file is used unmodified. This altered
version is not the original libpng; libpng is distributed under the libpng
license, which permits altered versions that are marked as such.

## curl

`curl/curl/curlbuild.h` is the build configuration header that curl's
configure step writes for a platform. Release archives ship a generic
placeholder; this one describes the Wii U's 32-bit PowerPC target. The other
curl 7.38.0 headers are used unmodified.
