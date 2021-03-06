#include "paintarea.h"

#include <QMessageBox>

#define STARTING_X 10

//Padding from the right side of the window
#define RIGHT_SIDE_PADDING 10
#define LIST_PADDING 50
#define STARTING_Y 10

PaintArea::PaintArea(QWidget *parent) :
    QWidget(parent)
{
    currentX = STARTING_X;
    currentY = STARTING_Y;
    totalWidth = 0;

    webpage = new Document;
    positionSet = false;

    currentCharacter = new QString;

    indentOn = false;
    nextWordChecked = false;
}

PaintArea::~PaintArea()
{
    delete webpage;
    delete currentCharacter;
}

void PaintArea::setDocument(Document *documentToSet)
{
    webpage = documentToSet;
}

void PaintArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (webpage->getFirstNode() != NULL)
    {

        QPainter qPainter(this);
        paintNodesVector = webpage->getFirstNode()->getPaintNodes();
        drawDocument(&qPainter, paintNodesVector);

        //Prevents document from moving around while being redrawn.
        positionSet = true;

        //This is necessary to show the entire paint area in the scroll area.
        setMinimumHeight(currentY);
        setMaximumHeight(currentY);
    }
}

void PaintArea::drawDocument(QPainter *qPainter,
                             std::vector<PaintNode*> *paintNodes)
{
    std::vector<PaintNode*>::iterator i = paintNodes->begin();

    for (; i != paintNodes->end(); i++)
    {
        paintCurrentNode(*i, qPainter, paintNodes, i);
    }
}

void PaintArea::insertLineBreak()
{
    QFont font = currentFont;
    QFontMetrics fm(font);

    if (!indentOn)
    {
        currentX = STARTING_X;
        currentY += 2 * fm.height();
        totalWidth = 0;
    }

    else
    {
        currentX = STARTING_X + LIST_PADDING;
        currentY -= fm.height();
        totalWidth = 0;
    }
}

void PaintArea::paintCurrentNode(PaintNode *currentPaintNode,
                                 QPainter *qPainter,
                                 std::vector<PaintNode*> *paintNodes,
                                 std::vector<PaintNode*>::iterator
                                 currentLocation)
{
    if (currentPaintNode->getTypeOfPaintNode() == "char")
    {
        //Draw the text contained within each paragraph node. New lines are
        //only added after each paragraph node--not any other element.
        if (positionSet)
        {
            updateCurrentPosition();
        }

        char *character = currentPaintNode->getCharacter();
        *currentCharacter = QString(*character);

        currentFont = qPainter->font();

        if (currentPaintNode->getWeight() == QFont::Bold)
        {
            currentFont.setBold(true);
        }
        else
        {
            currentFont.setBold(false);
        }

        if (currentPaintNode->getStyle() == QFont::StyleItalic)
        {
            currentFont.setItalic(true);
        }
        else
        {
            currentFont.setItalic(false);
        }

        QFontMetrics fm(currentFont);

        //Wrap text by entire words, not character-by-character.
        if (!nextWordChecked)
        {
            int currentLineWidth = totalWidth;
            currentLineWidth += getNextWordWidth(paintNodes, qPainter,
                                                 currentLocation);

            if (currentLineWidth + RIGHT_SIDE_PADDING + STARTING_X
                    >= this->width())
            {
                totalWidth = 0;
                currentY += fm.height();
                currentX = STARTING_X;
            }

            nextWordChecked = true;
        }

        QRect box(QPoint(currentX, currentY), QSize(fm.width(*character),
                                                    fm.height()));

        qPainter->setFont(currentFont);
        qPainter->drawText(box, Qt::AlignCenter, QString(*character));

        if (isspace(*character))
        {
            nextWordChecked = false;
        }

        updateCurrentPosition();
    }

    else if (currentPaintNode->getTypeOfPaintNode() == "image")
    {
        //Draw the image.
        QImage image(QString::fromStdString(currentPaintNode->getSourcePath()));
        if (image.width() + totalWidth >= this->width() + RIGHT_SIDE_PADDING)
        {
            totalWidth = 0;
            currentX = 0;

            QFontMetrics fm(currentFont);
            currentY += fm.height();
        }
        qPainter->drawImage(currentX, currentY, image);

        currentX += image.width() + 10;
        totalWidth += image.width() + 10;
        if (currentX >= this->width() - RIGHT_SIDE_PADDING)
        {
            currentX = STARTING_X;
        }
        currentY += image.height() + 10;
        nextWordChecked = false;

        currentLocation++;
        if ((*currentLocation)->getTypeOfPaintNode() == "node")
        {
            currentX = STARTING_X;
            totalWidth = 0;
        }
    }

    else if (currentPaintNode->getTypeOfPaintNode() == "hr")
    {
        QFontMetrics fm(currentFont);
        if (currentX != STARTING_X)
        {
            currentX = STARTING_X;
            currentY += fm.height();
        }

        qPainter->drawLine(QPoint(currentX, currentY),
                           QPoint(this->width() - RIGHT_SIDE_PADDING,
                                  currentY));

        currentY += fm.height();
        totalWidth = 0;
    }

    else if (currentPaintNode->getTypeOfPaintNode() == "ul")
    {
        //This is an ugly, hard-coded solution that will be removed as part of the
        //0.2.0 beta code rewrite and clean up.
        insertLineBreak();
        currentY -= 20;
        drawDocument(qPainter, currentPaintNode->returnNode()->
                     getPaintNodes());
    }

    else if (currentPaintNode->getTypeOfPaintNode() == "li")
    {
        indentOn = true;
        insertLineBreak();
        QPen bulletPoint(Qt::black);
        bulletPoint.setCapStyle(Qt::RoundCap);
        bulletPoint.setWidth(5);
        qPainter->setPen(bulletPoint);
        qPainter->drawPoint(currentX - 7, currentY + 10);
        drawDocument(qPainter, currentPaintNode->returnNode()->
                     getPaintNodes());
        indentOn = false;
        insertLineBreak();
    }

    else if (currentPaintNode->getTypeOfPaintNode() == "node")
    {

        //Call the function again on each of the PaintNode's child paint nodes.
        //This ensures that all of the child nodes of the overall parent node
        //are drawn.
        std::vector<PaintNode*> *childPaintNodes = currentPaintNode->
                returnNode()->getPaintNodes();
        drawDocument(qPainter, childPaintNodes);
        if (currentPaintNode->returnNode()->getTypeOfRenderNode() == "p")
        {
            insertLineBreak();
        }
    }
}

//This adjusts the location where the next character will be drawn, including
//changing lines when the total length exceeds the width of the window.
void PaintArea::updateCurrentPosition()
{

    //positionSet is only true when the entire document has been drawn.
    //If it is false, then it is safe to set the character to the spot
    //after the current character.
    if (!positionSet)
    {
        QFontMetrics fm(currentFont);

        totalWidth += fm.width(*currentCharacter);
        currentX += fm.width(*currentCharacter);
    }

    //However, if positionSet is true, then it is time to draw the entire
    //document again, so we reset the current position to the initial state.
    else
    {
        currentX = STARTING_X;
        currentY = STARTING_Y;
        totalWidth = 0;
        positionSet = false;
    }
}

int PaintArea::getNextWordWidth(std::vector<PaintNode*> *paintNodes,
                                QPainter *qPainter,
                                std::vector<PaintNode*>::iterator
                                currentNode)
{
    int wordWidth = 0;
    bool wordEndReached = false;

    for (; !wordEndReached; currentNode++)
    {
        if (currentNode == paintNodes->end() - 1)
        {
            wordEndReached = true;
        }

        if ((*currentNode)->getTypeOfPaintNode() == "char")
        {
            if (!isspace(*(*currentNode)->getCharacter()))
            {
                QFontMetrics fm(currentFont);
                wordWidth += fm.width((*currentNode)->getCharacter()[0]);
            }
            else
            {
                wordEndReached = true;
            }
        }

        else if ((*currentNode)->getTypeOfPaintNode() == "node")
        {
            wordWidth += getNextWordWidth(paintNodes, qPainter, currentNode);
        }
    }

    return wordWidth;
}
