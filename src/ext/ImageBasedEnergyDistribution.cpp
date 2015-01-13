#include <algorithm>
#include <stdexcept>

#include "World/ImageBasedEnergyDistribution.h"
#include "Utilities/SDL_gfxRoborobo.h"

ImageBasedEnergyDistribution::ImageBasedEnergyDistribution(std::string filename) {
    this->imageFilename = filename;
}

ImageBasedEnergyDistribution::~ImageBasedEnergyDistribution() {
}

void ImageBasedEnergyDistribution::load() {
    SDL_Surface *image = load_image(imageFilename);

    if (!image) {
        std::stringstream msg;
        msg << "\n\n####Could not load energy density image '" << imageFilename << "'!";
        throw std::runtime_error(msg.str());
    }
    std::cout<<"\n\n####IMAGE LOADED:"<<imageFilename<<"\n\n";
    distrib.clear();

    for (int x = 0; x < image->w; x++) {
        for (int y = 0; y < image->h; y++) {
            Uint32 pixel = getPixel32(image, x, y);
            Uint8 r, g, b;
            SDL_GetRGB(pixel, image->format, &r, &g, &b);

            if (r != 255) {
                Point2d point(x, y);
                distrib.addItem(point, 255-r);
            }
        }
    }

    SDL_FreeSurface(image);
}

Point2d ImageBasedEnergyDistribution::draw() {
    if (distrib.size() == 0) {
        throw std::runtime_error("Attempted to use ImageBasedEnergyDistribution before it was loaded!");
    }

    return distrib.draw();
}
