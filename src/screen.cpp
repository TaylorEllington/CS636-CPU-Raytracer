#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "screen.h"
#include "pixel.h"

#include <iostream>



Screen::Screen(int height, int width, Pixel backgroundColor, bool debugPurple):
    mHeight(height), mWidth(width), mDebugPurple(debugPurple)
{
    std::cout << "Screen - Setting up image as " << mWidth << " by " << mHeight;

    if (mDebugPurple) {
        std::cout << " with debug mode on" << std::endl;
        mImage.resize(mHeight * mWidth, { 238, 51, 238 });
    }
    else {
        std::cout << std::endl;
        mImage.resize(mHeight * mWidth, backgroundColor);
    }


}

void Screen::PrintImage(std::string filename) {
    // Not sure this needs to call an entierly seperate private function
    // but we are going to keep this just in case we have to add some kind
    // final proccessing

    Write(filename);

}

std::vector<Pixel>& Screen::getImage()
{
    return mImage;
}



void Screen::Write(std::string filename) {

    std::cout << "Screen - Writing " << mImage.size() << " pixels to "
        << mWidth << " by " << mHeight << " file: " << filename << std::endl;

    stbi_write_png(filename.c_str(), mWidth, mHeight, 3, mImage.data(), sizeof(Pixel) * mWidth);

}