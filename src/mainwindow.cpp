#include "mainwindow.h"
#include "modfinder.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    searchModsDialog = new SearchModsDialog(this);

    connect(ui->addModButton, &QPushButton::pressed, this,
            [&]() { searchModsDialog->show(); });
}

void MainWindow::OpenModpack(const QString &path)
{
    qDebug() << "Opening a modpack";
}

MainWindow::~MainWindow() { delete ui; }
