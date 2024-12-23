# Wow client
Wow client targeting wow TBC 2.4.3 8606

## dependencies
| name  | purpose |
| ------------ | --- |
| [zlib](http://zlib.net) | compression / decompression of zlib data |
| [libpng](http://www.libpng.org/pub/png/libpng.html) | PNG icon reading |
| [freetype](https://freetype.org) | font rasterizing |
| [lua](https://www.lua.org) | interfaces lua interpreter |
| [libxml2](http://www.xmlsoft.org/libxslt/index.html) | interfaces xml parsing |
| [jks](https://git.sr.ht/~acazuc/jks) | standard collections, mathematic helpers |
| [gfx](https://git.sr.ht/~acazuc/gfx) | OpenGL / D3D abstraction layer for rendering, xlib / win32 / glfw abstraction layer for window management |
| [libwow](https://git.sr.ht/~acazuc/libwow) | wow files parsing |
| [portaudio](http://portaudio.com) | audio interface |
| [libsamplerate](http://libsndfile.github.io/libsamplerate) | audio PCM resampling |
| [jkssl](https://git.sr.ht/~acazuc/jkssl) | cryptography (bignumbers, hash) |
| [jkl](https://git.sr.ht/~acazuc/jkl) | library compilation toolchain |

## how to build
- `cp config.sample config`
- `make lib`
- `make`

## thanks
none of this would exist without the incredible work done by the [wowdev](https://wowdev.wiki) contributors
