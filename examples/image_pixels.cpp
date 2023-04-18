

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>

#include <iostream>


int main()
{
    dlp::ReturnCode ret;
    dlp::Image      rgb_image;
    dlp::PixelRGB   rgb_pixel;

    dlp::Image    mono_image;
    unsigned char mono_pixel;

    unsigned int image_columns;
    unsigned int image_rows;


    // Create Image objects with 400 rows and 300 columns
    ret = rgb_image.Create( 400,300,dlp::Image::Format::RGB_UCHAR);
    if(ret.hasErrors()){
        std::cout << "RGB image creation FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }

    ret = mono_image.Create(400,300,dlp::Image::Format::MONO_UCHAR);
    if(ret.hasErrors()){
        std::cout << "Monochrome image creation FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }


    // Retrieve the image dimensions
    ret = rgb_image.GetColumns(&image_columns);
    ret = rgb_image.GetRows(&image_rows);
    std::cout << "Image dimensions are " << image_columns << " x " << image_rows << std::endl;

    // Draw a color gradient (red to green) into the image
    std::cout << "Drawing the images..." << std::endl;
    for(int iCols = 0; iCols < image_columns; iCols++){
        // Create RGB pixel value
        rgb_pixel.r=0;
        rgb_pixel.g=255*iCols/image_columns;
        rgb_pixel.b=255-(255*iCols/image_columns);

        // Create mono pixel value
        mono_pixel = (255 * iCols) / image_columns; // 255 is maximum unsigned char value

        // Set the pixel value in the two images
        for(int iRows=0; iRows < image_rows; iRows++){
            ret = rgb_image.SetPixel( iCols, iRows, rgb_pixel);
            ret = mono_image.SetPixel( iCols, iRows, mono_pixel);
        }
    }

    std::cout << "Saving the images" << std::endl;
    ret = rgb_image.Save("rgb_gradient.bmp");
    if(ret.hasErrors()){
        std::cout << "RGB image save FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }

    ret = mono_image.Save("mono_gradient.bmp");
    if(ret.hasErrors()){
        std::cout << "Monochrome image save FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }


    // Retrieve the first, middle, and last pixels from the first row in both images
    // NOTE: Pixel address start at 0. The last address will be the dimension - 1
    ret = rgb_image.GetPixel(0,0,&rgb_pixel);
    if(ret.hasErrors()){
        std::cout << "Get pixel (0,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "rgb_image (0,0) pixel = R" << (int)rgb_pixel.r
                                         << " G" << (int)rgb_pixel.g
                                         << " B" << (int)rgb_pixel.b << std::endl;
    }

    ret = rgb_image.GetPixel(200,0,&rgb_pixel);
    if(ret.hasErrors()){
        std::cout << "Get pixel (200,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "rgb_image (200,0) pixel = R" << (int)rgb_pixel.r
                                         << " G" << (int)rgb_pixel.g
                                         << " B" << (int)rgb_pixel.b << std::endl;
    }

    ret = rgb_image.GetPixel(399,0,&rgb_pixel);
    if(ret.hasErrors()){
        std::cout << "Get pixel (399,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "rgb_image (399,0) pixel = R" << (int)rgb_pixel.r
                                         << " G" << (int)rgb_pixel.g
                                         << " B" << (int)rgb_pixel.b << std::endl;
    }


    ret = mono_image.GetPixel(0,0,&mono_pixel);
    if(ret.hasErrors()){
        std::cout << "Get pixel (0,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "mono_image (0,0) pixel = " << (int)mono_pixel << std::endl;
    }

    ret = mono_image.GetPixel(200,0,&mono_pixel);
    std::cout << "mono_image (200,0) pixel = " << (int)mono_pixel << std::endl;
    if(ret.hasErrors()){
        std::cout << "Get pixel (200,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "mono_image (200,0) pixel = " << (int)mono_pixel << std::endl;
    }

    ret = mono_image.GetPixel(399,0,&mono_pixel);
    std::cout << "mono_image (399,0) pixel = " << (int)mono_pixel << std::endl;
    if(ret.hasErrors()){
        std::cout << "Get pixel (399,0) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "mono_image (399,0) pixel = " << (int)mono_pixel << std::endl;
    }

    // GetPixel has protection against invalid addresses
    ret = mono_image.GetPixel(1000,1000,&mono_pixel);
    if(ret.hasErrors()){
        std::cout << "Get pixel (1000,1000) FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }
    else{
        std::cout << "mono_image (1000,1000) pixel = " << (int)mono_pixel << std::endl;
    }

    return(0);
}

