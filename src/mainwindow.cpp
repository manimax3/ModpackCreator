#include "mainwindow.h"
#include "modfinder.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->addModButton, &QPushButton::pressed, this, [&]() {
        ModFinder *finder = new ModFinder("immersive");
        finder->PerformSearch();
        const auto mods = finder->GetFoundMods();
        for (const auto &mod : mods) {
            qDebug() << mod.addonname.c_str();
        }
    });
}

void MainWindow::OpenModpack(const QString &path)
{
    qDebug() << "Opening a modpack";
}

MainWindow::~MainWindow() { delete ui; }
