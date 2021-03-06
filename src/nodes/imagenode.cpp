#include "imagenode.h"

ImageNode::ImageNode()
{
    setNeedsPainting(true);
    setTypeOfRenderNode("image");

    sourcePath = new std::string;
    altText = new std::string;
}

ImageNode::~ImageNode()
{
    delete sourcePath;
    delete altText;
}

std::string* ImageNode::getSourcePath()
{
    return sourcePath;
}

void ImageNode::setSourcePath(std::string path)
{
    *sourcePath = path;
}

std::string* ImageNode::getAltText()
{
    return altText;
}

void ImageNode::setAltText(std::string text)
{
    *altText = text;
}
