#pragma once
#include "cursemeta.h"
#include "modpack.h"

#include <QDialog>

namespace Ui {
class CreateModpackDialog;
}

class CreateModpackDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateModpackDialog(QWidget *parent = nullptr);
    ~CreateModpackDialog();

signals:
    void ModpackCreated(Modpack modpack);

private slots:
    void OkPressed() {}
    void CancelPressed() {}

    void McVersionSelected(const QString &version);
    void McVersionFound(const CurseMetaMcVersion &version);
    void ForgeVersionFound(CurseMetaForge forge);

private:
    Ui::CreateModpackDialog * ui;
    McVersionFinder *         mcVersionFinder;
    std::list<CurseMetaForge> forgeVersions;
};
