#pragma once
#include "modfinder.h"

#include <QDialog>
#include <string>

namespace Ui {
    class SearchModsDialog;
}

class SearchModsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchModsDialog(QWidget *parent = nullptr);
    ~SearchModsDialog();

signals:
    void ModsExport(std::list<CurseMetaMod> mods);

public slots:
    void Search();
    void AddFoundMond(const ModFinder::SearchData &data);

private:
    Ui::SearchModsDialog *ui;
    ModFinder *finder;

    void HandleAddModsRequest();
};
