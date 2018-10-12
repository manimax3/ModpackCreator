#include "mainwindow.h"
#include "modfinder.h"
#include "modlistwidgetitem.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>

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

    connect(createModpackDialog, &CreateModpackDialog::ModpackCreated, this,
            [this](const Modpack &modpack) { currentModpack = modpack; });

    connect(searchModsDialog, &SearchModsDialog::ModsExport, this,
            [this](std::list<CurseMetaMod> mods) {
                currentModpack.mods.insert(std::end(currentModpack.mods),
                                           std::begin(mods), std::end(mods));

                for (const auto &mod : mods) {
                    ui->listWidget->addItem(
                        new ModListWidgetItem(mod, ui->listWidget));
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

MainWindow::~MainWindow() { delete ui; }

void MainWindow::CreateModpack() { createModpackDialog->show(); }
