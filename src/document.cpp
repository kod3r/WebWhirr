#include <iostream>
#include <string>
#include "nodes/rendernode.h"
#include "document.h"

Document::Document()
{
    firstNode = new RenderNode;
    firstNode = NULL;
}

Document::~Document()
{
    delete firstNode;
}

RenderNode* Document::getFirstNode()
{
    return firstNode;
}

void Document::constructTree(RenderNode *childNode, RenderNode *parentNode)
{   
    //This has the potential to crash if parentNode is somehow passed as NULL.
    //I will work on this.
    if (firstNode == NULL && parentNode == NULL)
    {
        firstNode = childNode;
    }
    else
    {
        childNode->setParentNode(parentNode);
        parentNode->addChildNode(childNode);
    }
}

void Document::paintWebpage()
{
    try
    {
        if (firstNode == NULL)
        {
            throw "Error: Webpage is empty.";
        }

        RenderNode *iterator;
        iterator = firstNode;
    }
    catch (std::string error)
    {
        std::cout << error << std::endl;
    }
}
