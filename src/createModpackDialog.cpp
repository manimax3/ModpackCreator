#include "createModpackDialog.h"
#include "ui_createModpackDialog.h"

CreateModpackDialog::CreateModpackDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateModpackDialog)
    , mcVersionFinder(new McVersionFinder(this))
{
    ui->setupUi(this);

    connect(mcVersionFinder, &McVersionFinder::McVersionFound, this,
            &CreateModpackDialog::McVersionFound);
    connect(mcVersionFinder, &McVersionFinder::ForgeVersionFound, this,
            &CreateModpackDialog::ForgeVersionFound);

    connect(ui->mcVersionSelect,
            QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &CreateModpackDialog::McVersionSelected);

    mcVersionFinder->StartSearching();
}

void CreateModpackDialog::McVersionFound(const CurseMetaMcVersion &version)
{
    ui->mcVersionSelect->addItem(version.version.c_str());
    ui->mcVersionSelect->model()->sort(0); // Kinda stupid to sort everytime
}

void CreateModpackDialog::ForgeVersionFound(CurseMetaForge forge)
{
    forgeVersions.push_back(forge);
}

void CreateModpackDialog::McVersionSelected(const QString &version)
{
    ui->forgeVersionSelect->clear();
    const auto vers = version.toStdString();
    for (const auto &v : forgeVersions) {

        if (v.mcversion != vers)
            continue;

        if (v.recommended)
            ui->forgeVersionSelect->insertItem(
                0, (v.name + " (recommended)").c_str());
        else
            ui->forgeVersionSelect->addItem(v.name.c_str());
    }

    ui->forgeVersionSelect->setCurrentIndex(0);
}

CreateModpackDialog::~CreateModpackDialog()
{
    delete ui;
    delete mcVersionFinder;
}
