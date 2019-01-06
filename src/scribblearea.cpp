#include <QtWidgets>
#include "../include/scribblearea.h"


ScribbleArea::ScribbleArea(QWidget *parent) : QWidget(parent) {
    //setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    myPenWidth = 3;
    myPenColor = Qt::red;
}

bool ScribbleArea::openImage(const QString &fileName) {
    QImage loadedImage;
    if (!loadedImage.load(fileName)) return false;
    QSize newSize = loadedImage.size();
    //this->parentWidget()->parentWidget()->resize(newSize * 1.1025f);
    //this->parentWidget()->resize(newSize * 1.05f);
    this->resize(newSize);

    //resizeImage(&loadedImage, newSize);
    image = loadedImage;
    modified = false;
    update();
    return true;
}

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat) {
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    if (visibleImage.save(fileName, fileFormat)) {
        modified = false;
        return true;
    }
    else return false;
}

void ScribbleArea::setPenColor(const QColor &newColor) {
    myPenColor = newColor;
}

void ScribbleArea::setPenWidth(int newWidth) {
    myPenWidth = newWidth;
}

void ScribbleArea::clearImage() {
    image.fill(qRgb(255, 255, 255));
    modified = true;
    update();
    snakePoints.clear();
}

void ScribbleArea::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && editable == true) {
        lastPoint = event->pos();
        if (lastPoint.x() > 0 && lastPoint.x() < image.width() &&
            lastPoint.y() > 0 && lastPoint.y() < image.height() ) {
            snakePoints.push_back(Point(lastPoint.x(), lastPoint.y()));
        }
        scribbling = true;
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event ) {
    if ((event->buttons() & Qt::LeftButton) && scribbling ) {
        QPoint p = event->pos();
        if (p.x() > 0 && p.x() < image.width() &&
            p.y() > 0 && p.y() < image.height() ) {
            drawLineTo(p);
            Point p2 = Point(p.x(), p.y());
            if (p2.distance(snakePoints.back()) > 4.0f) {
                snakePoints.push_back(p2);
            }
        }
    }
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && scribbling) {
        QPoint p = event->pos();
        if (p.x() > 0 && p.x() < image.width() &&
            p.y() > 0 && p.y() < image.height() ) {
            drawLineTo(p);
            Point p2 = Point(p.x(), p.y());
            if (p2.distance(snakePoints.back()) > 4.0f) {
                snakePoints.push_back(p2);
            }
        }
        scribbling = false;
    }
}

void ScribbleArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image, dirtyRect);
}

void ScribbleArea::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

void ScribbleArea::drawLineTo(const QPoint &endPoint) {
    QPainter painter(&image);
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);
    modified = true;

    int rad = (myPenWidth / 2) + 2;
    update(QRect(lastPoint, endPoint).normalized()
                                     .adjusted(-rad, -rad, +rad, +rad));
    lastPoint = endPoint;
}

void ScribbleArea::drawPoints() {
    lastPoint = QPoint(snakePoints[0].x, snakePoints[0].y);
    for (size_t i = 1; i < snakePoints.size(); i++) {
        drawLineTo(QPoint(snakePoints[i].x, snakePoints[i].y ));
    }
    drawLineTo(QPoint(snakePoints[0].x, snakePoints[0].y));
}

void ScribbleArea::resizeImage(QImage *image, const QSize &newSize) {
    if (image->size() == newSize) return;
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}
