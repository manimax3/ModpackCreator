#pragma once
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

public slots:
    void Search();

private:
    Ui::SearchModsDialog *ui;
};
