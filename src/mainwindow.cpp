#include <QtWidgets>
#include "../include/mainwindow.h"
#include "../include/cPoint.h"
#include "../include/cSnake.h"
#include "../include/gradflow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QGraphicsScene"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createActions();
    createMenus();

    connect(ui->pBtnDraw, SIGNAL (toggled(bool)), this, SLOT (drawSnake(bool)));
    connect(ui->pBtnAdapt, SIGNAL (clicked()), this, SLOT (adapt()));

    setWindowTitle(tr("Snake - Active Contour"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) event->accept();
    else event->ignore();
}

void MainWindow::open() {
    if (maybeSave()) {
        QString selfilter = tr("PPM BITMAP (*.ppm)");
        ImgFileName = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                QDir::currentPath(),
                tr("PPM BITMAP (*.ppm)"),
                &selfilter
        );
        if (!ImgFileName.isEmpty())
            ui->scribbleArea->openImage(ImgFileName);
    }
}

void MainWindow::save() {
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    saveFile(fileFormat);
}

void MainWindow::penColor() {
    QColor newColor = QColorDialog::getColor(ui->scribbleArea->penColor());
    if (newColor.isValid()) ui->scribbleArea->setPenColor(newColor);
}

void MainWindow::penWidth() {
    bool ok;
    int newWidth = QInputDialog::getInt(this, tr("Snake - Active Contour"),
                                        tr("Select pen width:"),
                                        ui->scribbleArea->penWidth(),
                                        1, 50, 1, &ok);
    if (ok) ui->scribbleArea->setPenWidth(newWidth);
}


void MainWindow::about() {
    QMessageBox::about(this, tr("About Scribble"),
            tr("<p><b>Snake - Active Contour</b> demonstrates the active contour algorithm.</p>"));
}

void MainWindow::createActions() {
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    foreach (QByteArray format, QImageWriter::supportedImageFormats()) {
        if (QString(format) == "ppm"){
            QString text = tr("%1...").arg(QString(format).toUpper());
            QAction *action = new QAction(text, this);
            action->setData(format);
            connect(action, SIGNAL(triggered()), this, SLOT(save()));
            saveAsActs.append(action);
        }
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    penColorAct = new QAction(tr("&Pen Color..."), this);
    connect(penColorAct, SIGNAL(triggered()), this, SLOT(penColor()));

    penWidthAct = new QAction(tr("Pen &Width..."), this);
    connect(penWidthAct, SIGNAL(triggered()), this, SLOT(penWidth()));

    clearScreenAct = new QAction(tr("&Clear Screen"), this);
    clearScreenAct->setShortcut(tr("Ctrl+L"));
    connect(clearScreenAct, SIGNAL(triggered()), ui->scribbleArea, SLOT(clearImage()));

    saveIntSteps = new QAction(tr("Save &Int Steps"), this);
    saveIntSteps->setCheckable(true);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus() {
    saveAsMenu = new QMenu(tr("&Save As"), this);
    foreach (QAction *action, saveAsActs)
        saveAsMenu->addAction(action);

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionMenu = new QMenu(tr("&Options"), this);
    optionMenu->addAction(penColorAct);
    optionMenu->addAction(penWidthAct);
    optionMenu->addSeparator();
    optionMenu->addAction(clearScreenAct);
    optionMenu->addSeparator();
    optionMenu->addAction(saveIntSteps);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(optionMenu);
    menuBar()->addMenu(helpMenu);
}

bool MainWindow::maybeSave() {
    if (ui->scribbleArea->isModified()) {
       QMessageBox::StandardButton ret;
       ret = QMessageBox::warning(this, tr("Snake - Active Contour"),
                          tr("The image has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) return saveFile("png");
        else if (ret == QMessageBox::Cancel) return false;
    }
    return true;
}

bool MainWindow::saveFile(const QByteArray &fileFormat) {
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                               initialPath,
                               tr("%1 Files (*.%2);;All Files (*)")
                               .arg(QString::fromLatin1(fileFormat.toUpper()))
                               .arg(QString::fromLatin1(fileFormat)));
    if (fileName.isEmpty()) return false;
    else return ui->scribbleArea->saveImage(fileName, fileFormat.constData());
}

void MainWindow::drawSnake(bool state) {
    ui->scribbleArea->editable = state;
}

void MainWindow::adapt() {
    penColor();
    cImage<> inImg = cImage<>(ImgFileName.toStdString());
    cImage<uint8_t> grdImg;
    cImage<uint8_t> flwImg;

    Array2D<uint8_t> gradient = Array2D<uint8_t>(inImg.columns, inImg.rows);
    Array2D<uint8_t> flow = Array2D<uint8_t>(inImg.columns, inImg.rows);

    getFlow(inImg, gradient, flow, ui->sBoxThreshold->value(), saveIntSteps->isChecked());

    if (saveIntSteps->isChecked()) {
        grdImg = cImage<uint8_t>(gradient.arr, gradient.cols, gradient.rows);
        flwImg = cImage<uint8_t>(flow.arr, flow.cols, flow.rows);

        grdImg.write("./output/" + inImg.getFileName() + "_sobel.pgm");
        flwImg.write("./output/" + inImg.getFileName() + "_flow.pgm");
    }

    Snake snake = Snake(gradient, flow, ui->scribbleArea->snakePoints);

    snake.alpha = ui->sBoxAlpha->value();
    snake.beta  = ui->sBoxBeta->value();
    snake.gamma = ui->sBoxGamma->value();
    snake.delta = ui->sBoxDelta->value();

    ui->scribbleArea->snakePoints = snake.adaptPoints();
    ui->scribbleArea->drawPoints();


    return;
}
