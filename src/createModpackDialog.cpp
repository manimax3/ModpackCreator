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

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
            &CreateModpackDialog::OkPressed);

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
                0, (v.name + " (recommended)").c_str(),
                QVariant(QString(v.name.c_str())));
        else
            ui->forgeVersionSelect->addItem(v.name.c_str(),
                                            QVariant(QString(v.name.c_str())));
    }

    ui->forgeVersionSelect->setCurrentIndex(0);
}

void CreateModpackDialog::OkPressed() {
    const auto author    = ui->authorEdit->text();
    const auto name      = ui->nameEdit->text();
    const auto version   = ui->versionEdit->text();
    const auto mcversion = ui->mcVersionSelect->currentText();
    const auto fversion  = ui->forgeVersionSelect->currentData().toString();

    if (author.isEmpty() || name.isEmpty() || version.isEmpty())
        // TODO: Maybe we should let the user know
        return;

    Modpack modpack;
    modpack.author       = author.toStdString();
    modpack.name         = name.toStdString();
    modpack.version      = version.toStdString();
    modpack.mcversion    = mcversion.toStdString();
    modpack.forgeversion = fversion.toStdString();

    emit ModpackCreated(modpack);
}

CreateModpackDialog::~CreateModpackDialog()
{
    delete ui;
    delete mcVersionFinder;
}
