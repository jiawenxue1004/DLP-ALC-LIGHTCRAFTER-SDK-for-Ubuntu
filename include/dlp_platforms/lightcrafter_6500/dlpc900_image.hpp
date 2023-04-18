#ifndef SPLASH_H
#define SPLASH_H

#include <dlp_platforms/lightcrafter_6500/common.hpp>

typedef struct splashhdr
{
    uint32  Signature;      /**< format 3 == "Splc" */
                            /**< (0x53, 0x70, 0x6c, 0x63) */
    uint16  Image_width;    /**< width of image in pixels */
    uint16  Image_height;   /**< height of image in pixels */
    uint32  Byte_count;     /**< number of bytes starting at "data" */
    uint32  Subimg_offset;  /**< byte-offset from "data" to 1st line */
                            /**< of sub-image, or 0xFFFFFFFF if none. */
    uint32  Subimg_end;     /**< byte-offset from "data" to end of */
                            /**< last line of sub-image, */
                            /**< or 0xFFFFFFFF if none. */
    uint32  Bg_color;       /**< unpacked 24-bit background color */
                            /**< (format: 0x00RRGGBB) */
    uint8  Pixel_format;   /**< format of pixel data */
                            /**< 0 = 24-bit unpacked: 0x00RRGGBB Not supported by DDP2230/DDPDDP243x*/
                            /**< 1 = 24-bit packed:   RGB 8-8-8 */
                            /**< 2 = 16-bit:          RGB 5-6-5   DDP3020 only */
                            /**< 3 = 16-bit:          YCrCb 4:2:2 DDP2230/DDPDDP243x only */
    uint8  Compression;    /**< compression of image */
                            /**< SPLASH_FORCE_UNCOMPRESSED  = uncompressed */
                            /**< SPLASH_FORCE_RLE           = RLE compressed */
                            /**< SPLASH_USER_DEFINED        = User Defined Compression */
                            /**< SPLASH_FORCE_RLE_2PIXEL    = RLE compressed 2Pixel */

    uint8  ByteOrder;      /**< 0 - pixel is 00RRGGBB - DDP3020 */
                            /**< 1 - pixel is 00GGRRBB - DDP2230/DDPDDP243x */
    uint8  ChromaOrder;    /**< Indicates chroma order of pixel data (DDP2230/DDPDDP243x only) */
                            /**< 0 - Cr is first pixel (0xYYRR) */
                            /**< 1 - Cb is first pixel (0xYYBB) */
    uint8  Is3D;               /**< 0 = 2D, this image is independent */
                                /**< 1 = 3D, this image is part of a 2 image eye pair */

    uint8  IsLeftImage;        /**< 0 = if Is3D=1, this is the Right Image of a 2 image eye pair */
                                /**< 1 = if Is3D=1, this is the Left Image of a 2 image eye pair */

    uint8  IsVertSubSampled;   /**< 0 = image is not vertically sub-sampled */
                                /**< 1 = image is vertically sub-sampled */

    uint8  IsHorzSubSampled;   /**< 0 = image is not horizontally sub-sampled */
                                /**< 1 = image is horizontally sub-sampled */

    uint8  IsSPCheckerboard;   /**< 0 = image is normal orthogonal image */
                                /**< 1 = image is Smooth Picture(tm) pre-merged checkerboard image */

    uint8  ChromaSwap;         /**< Indicates whether YUV source has chroma channels inverted */
                                /**< 0 = Source chroma channels are inverted. */
                                /**< 1 = Source chroma channels are not inverted.*/

    uint8  Pad[14];             /**< pad so that data starts at 16-byte boundary */    
} SPLASH_HEADER;

typedef struct compressedimage
{
    unsigned char *bitmapImage1;
    int sizeBitmap1;
    unsigned char *bitmapImage2;
    int sizeBitmap2;
} COMPRESSED_BITMAPIMAGES;

int Splash_createImage(unsigned char *pImageBuffer, COMPRESSED_BITMAPIMAGES *images, int *compression, bool split);
#endif
