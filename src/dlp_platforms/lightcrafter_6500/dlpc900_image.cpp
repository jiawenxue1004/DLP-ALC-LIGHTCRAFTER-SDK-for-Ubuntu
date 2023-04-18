
#include <dlp_platforms/lightcrafter_6500/dlpc900_image.hpp>

#ifdef _WIN32
#include <windows.h>

#else
typedef short WORD;
typedef long LONG;
typedef uint32 DWORD;
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    WORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    WORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#endif


static int SPLASH_PerformLineCompression(unsigned char *SourceAddr, int ImageWidth, int ImageHeight, uint32 *compressed_size, uint8 numLines)
{
    uint16 Row, Col;
    uint32 lineLength, bytesPerPixel = 3, imageHeight, pixelIndex;
    unsigned char *line1Data, *line2Data;

    lineLength =  ImageWidth * bytesPerPixel;

    if(lineLength % 4 != 0)
    {
        lineLength = (lineLength / 4 + 1) * 4;
    }

    if(numLines > 8)
    {
        *compressed_size = 0xFFFFFFFF;
        return 1;
    }

    imageHeight = (ImageHeight / numLines) * numLines;

    for (Row = 0; Row < (imageHeight - numLines); Row++)
    {
        line1Data = (unsigned char *)(SourceAddr + (lineLength *  Row));
        line2Data = (unsigned char *)(SourceAddr + (lineLength * (Row + numLines)));

        for (Col = 0; Col < ImageWidth; Col++)
        {
            pixelIndex = Col * bytesPerPixel;

            if( (line1Data[pixelIndex  ] != line2Data[pixelIndex  ]) ||
                    (line1Data[pixelIndex+1] != line2Data[pixelIndex+1]) ||
                    (line1Data[pixelIndex+2] != line2Data[pixelIndex+2]))
            {
                *compressed_size = 0xFFFFFFFF;
                return 1;
            }
        }
    }

    *compressed_size = numLines * lineLength;
    return 0;
}

static int SPLASH_PerformRLECompression(unsigned char *SourceAddr, unsigned char *DestinationAddr, int ImageWidth, int ImageHeight, uint32 *compressed_size)
{
    uint16 Row, Col;
    BOOL   FirstPixel = TRUE;
    uint8 Repeat = 1;
    uint32 Pixel = 0, S, D;
    uint32 LastColor = 0, pad;
    uint8 count = 0;

    //    uint8 *SourceAddr		= (uint08 *)(SplashRLECfg->SourceAddr);
    //    uint8 *DestinationAddr = (uint08 *)(SplashRLECfg->DestinationAddr);
    uint32 PixelSize		= 3;
    
    S = 0;
    D = 0;

    /* RLE Encode the Splash Image*/
    for (Row = 0; Row < ImageHeight; Row++)
    {
        for (Col = 0; Col < ImageWidth; Col++)
        {

            memcpy(&Pixel, SourceAddr + S, PixelSize);

            /* if this is the first Pixel, remember it and move on... */
            if (FirstPixel)
            {
                LastColor  = Pixel;
                Repeat  = 1;
                FirstPixel = FALSE;
                count = 0;
            }
            else
            {
                if (Pixel == LastColor)
                {
                    if (count)
                    {
                        if(count > 1) DestinationAddr[D++] = 0;
                        DestinationAddr[D++] = count;
                        memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 1) * PixelSize)) , count * PixelSize);
                        D += (count * PixelSize);
                        count = 0;
                    }

                    Repeat++;

                    if (Repeat == 255)
                    {
                        DestinationAddr[D++] = Repeat;
                        memcpy(DestinationAddr + D, &LastColor, PixelSize);
                        D += PixelSize;
                        FirstPixel = TRUE;
                    }
                }
                else
                {
                    if (Repeat == 1)
                    {
                        count++;
                        if (count == 255)
                        {
                            if(count > 1) DestinationAddr[D++] = 0;
                            DestinationAddr[D++] = count;
                            memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 1) * PixelSize)), count * PixelSize);
                            D += (count * PixelSize);
                            count = 0;
                        }
                    }
                    else
                    {
                        DestinationAddr[D++] = Repeat;
                        memcpy(DestinationAddr + D, &LastColor, PixelSize);
                        D += PixelSize;
                        Repeat = 1;
                    }
                    LastColor = Pixel;
                }
            }

            /* Last Pixel of the line*/
            if (Col == (ImageWidth-1) && Repeat != 255 && count != 255)
            {
                if (count)
                {
                    DestinationAddr[D++] = 0;
                    DestinationAddr[D++] = count + 1;
                    memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 2) * PixelSize)), (count + 1) * PixelSize);
                    D += ((count + 1) * PixelSize);
                    count = 0;
                }
                else
                {
                    DestinationAddr[D++] = Repeat;
                    memcpy(DestinationAddr + D, &LastColor, PixelSize);
                    D += PixelSize;
                }
                FirstPixel = TRUE;
            }
            S += PixelSize;

        }
        // END OF LINE
        DestinationAddr[D++] = 0;
        DestinationAddr[D++] = 0;

        /* Scan lines are always padded out to next 32-bit boundary */
        if(D % 4 != 0)
        {
            pad = 4 - (D % 4);
            memset(DestinationAddr + D, 0, pad);
            D += pad;
        }
    }

    /* End of file: Control Byte = 0 & Color Byte = 1 */
    DestinationAddr[D++] = 0;
    DestinationAddr[D++] = 1;

    /* End of file should be padded out till 128-bit boundary */
    if(D % 16 != 0)
    {
        pad = 16 - (D % 16);
        memset(DestinationAddr + D, 0, pad);
        D += pad;
    }

    // update flash size
    *compressed_size = D;

    return 0;
}

static int SPLASH_PerformRLESplCompression(unsigned char *SourceAddr, unsigned char *DestinationAddr, int ImageWidth, int ImageHeight, uint32 *compressed_size)
{
    int row, col, raw, destPtr = 0;

    for (row = 0; row < ImageHeight; row++)
    {
        col = 0;
        raw = 0;
        int rowSize =  (row * ImageWidth * 3);
        int prevRowSize  = ((row - 1) * ImageWidth * 3);

        while (col < ImageWidth)
        {
            int i = col;
            int repeat = 0, copy = 0;

            uint32 old = 0;

            memcpy(&old, SourceAddr + rowSize + (i * 3), 3);
            i++;
            while(i < ImageWidth)
            {
                uint32 pixel = 0;
                memcpy(&pixel, SourceAddr + rowSize + (i * 3), 3);

                if(old != pixel)
                    break;
                i++;
            }

            repeat = i - col;

            i = col;

            if(row != 0)
            {
                while(i < ImageWidth)
                {
                    uint32 prevRowPixel = 0, curRowPixel = 0;

                    memcpy(&prevRowPixel, SourceAddr + prevRowSize + (i * 3), 3);
                    memcpy(&curRowPixel, SourceAddr + rowSize + (i * 3), 3);

                    if (prevRowPixel != curRowPixel)
                        break;
                    i++;
                }
                copy = i - col;
            }

            if(copy > 0 && copy >= repeat)
            {
                if(raw > 1)
                {
                    DestinationAddr[destPtr++] = 0;
                    if (raw < 128)
                    {
                        DestinationAddr[destPtr++] = raw;
                    }
                    else
                    {
                        DestinationAddr[destPtr++] = raw | 0x80;
                        DestinationAddr[destPtr++] = (raw >> 7) & 0xFF;

                    }
                    memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                    destPtr+= raw * 3;
                }
                else if (raw)
                {
                    DestinationAddr[destPtr++] = raw;
                    memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                    destPtr+= raw * 3;
                }

                DestinationAddr[destPtr++] = 0;
                DestinationAddr[destPtr++] = 1;
                if (copy < 128)
                {
                    DestinationAddr[destPtr++] = copy;
                }
                else
                {
                    DestinationAddr[destPtr++] = copy | 0x80;
                    DestinationAddr[destPtr++] = (copy >> 7) & 0xFF;
                }

                col += copy;
                raw = 0;
            }
            else if(repeat > 1)
            {
                if(raw > 1)
                {
                    DestinationAddr[destPtr++] = 0;
                    if (raw < 128)
                    {
                        DestinationAddr[destPtr++] = raw;
                    }
                    else
                    {
                        DestinationAddr[destPtr++] = raw | 0x80;
                        DestinationAddr[destPtr++] = (raw >> 7) & 0xFF;

                    }
                    memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                    destPtr+= raw * 3;
                }
                else if (raw)
                {
                    DestinationAddr[destPtr++] = raw;
                    memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                    destPtr+= raw * 3;
                }
                if (repeat < 128)
                {
                    DestinationAddr[destPtr++] = repeat;
                }
                else
                {
                    DestinationAddr[destPtr++] = repeat | 0x80;
                    DestinationAddr[destPtr++] = (repeat >> 7) & 0xFF;
                }
                memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + (col * 3), 3);
                destPtr+=  3;

                col += repeat;
                raw = 0;
            }
            else
            {
                col++;
                raw++;
            }
        }
        if (raw)
        {
            if(raw > 1)
            {
                DestinationAddr[destPtr++] = 0;
                if (raw < 128)
                {
                    DestinationAddr[destPtr++] = raw;
                }
                else
                {
                    DestinationAddr[destPtr++] = raw | 0x80;
                    DestinationAddr[destPtr++] = (raw >> 7) & 0xFF;

                }
                memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                destPtr+= raw * 3;
            }
            else if (raw)
            {
                DestinationAddr[destPtr++] = raw;
                memcpy(DestinationAddr + destPtr, SourceAddr + rowSize + ((col - raw) * 3), raw * 3);
                destPtr+= raw * 3;
            }
        }
    }

    DestinationAddr[destPtr++] = 0;
    DestinationAddr[destPtr++] = 1;
    DestinationAddr[destPtr++] = 0;
    *compressed_size = destPtr;
}

static int SPLASH_PerformRLEUnCompression(unsigned char *SourceAddr, unsigned char *DestinationAddr, uint32 *size)
{
    uint32 PixelSize= 3, S = 0, D = 0;
    int i;

    while (S < *size)
    {
        uint32 ctrl_byte, color_byte;

        ctrl_byte = SourceAddr[S];
        color_byte = SourceAddr[S + 1];
        if (ctrl_byte == 0)
        {
            if (color_byte == 1)	// End of image
                break;
            else if (color_byte == 0)	// End of Line.
            {
                i = 0;
                S +=2;
                if (S % 4 != 0)
                {
                    int pad = 4 - (S % 4);
                    S += pad;
                }
            }
            else if (color_byte >= 2)
            {
                S +=2;
                memcpy(DestinationAddr + D, SourceAddr + S, color_byte * PixelSize);
                D += color_byte * PixelSize;
                S += color_byte * PixelSize;
            }
            else
                return -1;
        }
        else if (ctrl_byte > 0)
        {
            S++;
            for (i = 0; i < ctrl_byte; i++)
            {
                memcpy(DestinationAddr + D, SourceAddr + S, PixelSize);
                D += PixelSize;
            }
            S += PixelSize;
        }
        else
            return -1;
    }
    *size = D;
    return 0;
}

static int Splash_compressImage(unsigned char *bitmapImage, COMPRESSED_BITMAPIMAGES *images, int *compression, int height, int width, int bytesPerPixel, bool first)
{
    uint32 splashSize;
    SPLASH_HEADER splash_header = {0};
    unsigned char *splashImage;

    unsigned char *rleBuffer = (unsigned char *)malloc((((height * width * bytesPerPixel) +
                                                         ((height * width * 4) / 255) + (width * 2) + 15) - 1));
    if (rleBuffer == NULL)
    {
        return -1;//ERROR_NO_MEM_FOR_MALLOC;
    }

    unsigned char *splRleBuffer = (unsigned char *)malloc((((height * width * bytesPerPixel) +
                                                            ((height * width * 4) / 255) + (width * 2) + 15) - 1));

    if (rleBuffer == NULL)
    {
        return -1;//ERROR_NO_MEM_FOR_MALLOC;
    }

    switch(*compression)
    {
    case 0: // force uncompress
        splashSize  = height * width * bytesPerPixel;
        splashImage = bitmapImage;
        break;

    case 1: // force rle compress
        SPLASH_PerformRLECompression(bitmapImage, rleBuffer, width, height, &splashSize);
        splashImage = rleBuffer;
        break;
    case 2:
        SPLASH_PerformRLESplCompression(bitmapImage, splRleBuffer, width, height, &splashSize);
        splashImage = splRleBuffer;
        break;

    case 4: // force 4 line compress
        splashSize  = 4 * width * bytesPerPixel;
        splashImage = bitmapImage;
        break;

    default: // auto compression
        uint32 lineCompSize, rleCompSize, splRleCompSize;

        SPLASH_PerformLineCompression(bitmapImage, width, height, &lineCompSize, 4);
        SPLASH_PerformRLECompression(bitmapImage, rleBuffer, width, height, &rleCompSize);
        SPLASH_PerformRLESplCompression(bitmapImage, splRleBuffer, width, height, &splRleCompSize);

        splashSize  = height * width * bytesPerPixel;

        /*		if(lineCompSize < splashSize) // 4 Line Compression not supported for HiRes
        {
            splashSize  = 4 * width * bytesPerPixel;;
            splashImage = bitmapImage;
            *compression = 4;
        }
        else*/ if (splRleCompSize < splashSize)
        {
            splashSize  = splRleCompSize;
            splashImage = splRleBuffer;
            *compression    = 2;
        }
        else if(rleCompSize < splashSize)
        {
            splashSize  = rleCompSize;
            splashImage = rleBuffer;
            *compression    = 1;
        }
        else
        {
            splashSize  = height * width * bytesPerPixel;
            splashImage = bitmapImage;
            *compression = 0;
        }

        break;
    }
    splash_header.Signature		= 0x646C7053;
    splash_header.Image_width	= (uint16)width;
    splash_header.Image_height	= (uint16)height;
    splash_header.Pixel_format	= 1; // 24-bit packed
    splash_header.Subimg_offset 	= -1;
    splash_header.Subimg_end	= -1;
    splash_header.Bg_color		= 0;
    splash_header.ByteOrder		= 1;
    splash_header.ChromaOrder	= 0;
    splash_header.Byte_count	= splashSize;
    splash_header.Compression	= *compression;
    splash_header.IsLeftImage	= 1;

    if (first)
    {
        images->bitmapImage1 = (unsigned char *) malloc (splashSize + sizeof(splash_header));
        memcpy(images->bitmapImage1, &splash_header, sizeof(splash_header));
        memcpy(images->bitmapImage1 + sizeof(splash_header), splashImage, splashSize);
        images->sizeBitmap1 = sizeof(splash_header) + splashSize;
    }
    else
    {
        images->bitmapImage2 = (unsigned char *) malloc (splashSize + sizeof(splash_header));
        memcpy(images->bitmapImage2, &splash_header, sizeof(splash_header));
        memcpy(images->bitmapImage2 + sizeof(splash_header), splashImage, splashSize);
        images->sizeBitmap2 = sizeof(splash_header) + splashSize;
    }

    free(rleBuffer);
    free(splRleBuffer);
    return 0;
}

int Splash_createImage(unsigned char *pImageBuffer, COMPRESSED_BITMAPIMAGES *images, int *compression, bool split)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER headerInfo;
    unsigned char *bitmapImage, *line1Data, *line2Data, *bitmapImage_left, *bitmapImage_right;
    int lineLength, i, j, splitImage_width, splitImage_height, bytesPerPixel, ret = 0;


    memcpy(&fileHeader, pImageBuffer, sizeof(fileHeader));
    memcpy(&headerInfo, pImageBuffer + sizeof(fileHeader), sizeof(headerInfo));

    if (fileHeader.bfType != 0x4D42)
        return -1;//ERROR_NOT_BMP_FILE;
    if(headerInfo.biBitCount != 24)
        return -2;//ERROR_NOT_24bit_BMP_FILE;

    bitmapImage = (unsigned char *)malloc(fileHeader.bfSize - fileHeader.bfOffBits);
    if (!bitmapImage)
        return -3;//ERROR_NO_MEM_FOR_MALLOC;

    memcpy(bitmapImage, pImageBuffer + fileHeader.bfOffBits, fileHeader.bfSize - fileHeader.bfOffBits);

    bytesPerPixel = headerInfo.biBitCount / 8;

    lineLength    =  headerInfo.biWidth * (bytesPerPixel);

    if(lineLength % 4 != 0)
    {
        lineLength = (lineLength / 4 + 1) * 4;
    }
    line1Data = (unsigned char *)malloc(lineLength);

    if(line1Data == NULL)
    {
        free(bitmapImage);
        return -4;//ERROR_NO_MEM_FOR_MALLOC;
    }

    line2Data = (unsigned char *)malloc(lineLength);

    if(line2Data == NULL)
    {
        free(line1Data);
        free(bitmapImage);
        return -5;//ERROR_NO_MEM_FOR_MALLOC;
    }

    // vertically flip the bitmap image
    for(i = 0; i < (headerInfo.biHeight / 2); i++)
    {
        memcpy(line1Data, bitmapImage + (lineLength * i), lineLength);
        memcpy(line2Data, bitmapImage + (lineLength * (headerInfo.biHeight - i - 1)), lineLength);

        unsigned char tempbyte;

        for(j = 0; j < headerInfo.biWidth; j++)
        {
            
            tempbyte = line1Data[j * 3 + 2];
            line1Data[j * 3 + 2] = line1Data[j * 3 + 1];
            line1Data[j * 3 + 1] = tempbyte;

            tempbyte = line2Data[j * 3 + 2];
            line2Data[j * 3 + 2] = line2Data[j * 3 + 1];
            line2Data[j * 3 + 1] = tempbyte;
        }

        memcpy(bitmapImage + (lineLength * (headerInfo.biHeight - i - 1)), line1Data, lineLength);
        memcpy(bitmapImage + (lineLength * i), line2Data, lineLength);
    }

    free(line1Data);
    free(line2Data);

    if(split)
    {
        splitImage_width = headerInfo.biWidth / 2;
        splitImage_height = headerInfo.biHeight;

        bitmapImage_left = (unsigned char *)malloc(splitImage_width * splitImage_height * (bytesPerPixel));
        bitmapImage_right = (unsigned char *)malloc(splitImage_width * splitImage_height * (bytesPerPixel));

        for (i = 0; i < headerInfo.biHeight; i++)
        {
            memcpy(bitmapImage_left + (i * splitImage_width * 3), bitmapImage + (i * headerInfo.biWidth * 3), splitImage_width * 3);
            memcpy(bitmapImage_right + (i * splitImage_width * 3), bitmapImage + (i * headerInfo.biWidth * 3) + splitImage_width * 3, splitImage_width * 3);
        }

        ret = Splash_compressImage(bitmapImage_left, images, compression, splitImage_height, splitImage_width, bytesPerPixel, true);
        if (!ret)
            ret = Splash_compressImage(bitmapImage_right, images, compression, splitImage_height, splitImage_width, bytesPerPixel, false);

        free(bitmapImage);
        free(bitmapImage_left);
        free(bitmapImage_right);
        return ret;
    }
    else
    {
        ret = Splash_compressImage(bitmapImage, images, compression, headerInfo.biHeight, headerInfo.biWidth, bytesPerPixel, true);
        free(bitmapImage);
        return ret;
    }
}
