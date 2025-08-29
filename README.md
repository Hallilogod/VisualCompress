# VisualCompress

**Compress any file into a PNG and back.**

VisualCompress is a simple CLI tool to convert files into viewable PNGs by encoding their bytes as RGB values for the pixels of the resulting image. 

**Example: Encode test.mp3 into test.png and decode test.png back to result.mp3**

```
    > visualcompress encode test.mp3 test.png
    Read 2305297 bytes from 'test.mp3' into memory
    Encoded 'test.mp3' to 'test.png'

    > visualcompress decode test.png result.mp3
    Read 2305116 bytes from 'test.png' into memory
    Decoded 'test.png' to 'result.mp3'
```

The lodepng.c/.h files are part of the [Lodepng project](https://github.com/lvandeve/lodepng).

The tool should be cross compilable (atleast we managed to compile it on windows and linux).

I love data.