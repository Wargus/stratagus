C++ implementation of hq2x and hq3x scaling algorithms. This implementation was made using [DOSBOX](http://www.dosbox.com/) and [hqxSharp](https://bitbucket.org/Tamschi/hqxsharp) as reference.

## Algorithm

The hqx algorithms ("hq" stands for "high quality" and "x" stands for magnification) is a scaling algorithms created by Maxim Stepin and it's mostly used in emulators.

The algorithm description from [Wikipedia](https://en.wikipedia.org/wiki/Hqx):

> First, the color of each of the 8 pixels around the source pixel is compared to the color of the source pixel. Shapes are detected by checking for pixels of similar color according to a threshold. This gives total of 28 = 256 combinations of similar or dissimilar neighbors. To expand the single pixel into a 2×2, 3×3, or 4×4 block of pixels, the arrangement of neighbors is looked up in a predefined table which contains the necessary interpolation patterns.

For more information, check out the archived version of the original Maxim Stepin website: [HiEnd3D - Demos & Docs](https://web.archive.org/web/20131105130843/http://www.hiend3d.com/demos.html).

## Versions

There are two "versions" of the HQx algorithm: one to produce a sharper output and another for a smoothed output. Check out the file `HQx.cc` for more information.

## Samples

Original test image:

![Original](samples/test-image.bmp)

Test image scaled by 2x (smoothed version):

![Original](samples/test-image-hq2x.bmp)

Test image scaled by 3x (smoothed version):

![Original](samples/test-image-hq3x.bmp)

Prince of Persia 1 screenshot:

![Original](samples/prince.bmp)

Prince of Persia 1 screenshot scaled by 2x (sharper version):

![Original](samples/prince-hq2x.bmp)
