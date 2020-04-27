#include <vector>
#include <string>
#include "pixel.h"



class Screen {

public:
    Screen(int height,int  width, Pixel backgroundColor, bool debugPurple);
    void PrintImage(std::string filename);
    std::vector<Pixel>& getImage();

private:

    void Write(std::string filename);


    int mHeight = 0;
    int mWidth = 0;
    
    bool mDebugPurple = false;

    std::vector<Pixel> mImage;


};