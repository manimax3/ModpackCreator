#include "mainwindow.h"
#include "modfinder.h"
#include "modlistwidgetitem.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <fstream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    searchModsDialog    = new SearchModsDialog(this);
    createModpackDialog = new CreateModpackDialog(this);

    connect(ui->addModButton, &QPushButton::pressed, this,
            [&]() { searchModsDialog->show(); });

    connect(ui->actionLoad, &QAction::triggered, this,
            &MainWindow::OpenModpack);

    connect(ui->actionCreate_New, &QAction::triggered, this,
            &MainWindow::CreateModpack);

    connect(ui->actionSave, &QAction::triggered, this,
            &MainWindow::SaveModpack);

    connect(createModpackDialog, &CreateModpackDialog::ModpackCreated, this,
            [this](const Modpack &modpack) { currentModpack = modpack; });

    connect(searchModsDialog, &SearchModsDialog::ModsExport, this,
            [this](std::list<CurseMetaMod> mods) {
                for (const auto &m : mods) {
                    currentModpack.AddMod(m);
                }

                ui->listWidget->clear();

                for (const auto &m : currentModpack.GetMods()) {
                    ui->listWidget->addItem(
                        new ModListWidgetItem(m, ui->listWidget));
                }
            });
}

void MainWindow::OpenModpack()
{
    const auto result
        = QFileDialog::getOpenFileName(this, "Please select the project file");

    if (result.isEmpty())
        return;

    QFile file(result);
    json  j;
    try {
        j = json::parse(file.readAll());
    } catch (const json::parse_error &e) {
        qWarning() << "Not a valid project file";
        return;
    }

    try {
        j.get_to(currentModpack);
    } catch (const json::parse_error &e) {
        qCritical() << "Couldnt parse the modpack file";
        return;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete searchModsDialog;
    delete createModpackDialog;
}

void MainWindow::CreateModpack() { createModpackDialog->show(); }

void MainWindow::SaveModpack()
{
    std::ofstream of(FILE_NAME);
    json          j = currentModpack;
    of << j;
}
