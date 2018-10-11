#pragma onc
#include <QMainWindow>

#include "searchModsDialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void OpenModpack(const QString &path);

private:
    Ui::MainWindow *  ui;
    SearchModsDialog *searchModsDialog;
};
