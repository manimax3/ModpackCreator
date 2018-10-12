#pragma onc
#include <QMainWindow>
#include <string_view>

#include "createModpackDialog.h"
#include "modpack.h"
#include "searchModsDialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static constexpr auto FILE_NAME = "modpack.json";

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void OpenModpack();
    void ImportModpack() {}
    void CreateModpack();
    void ExportModpack() {}
    void SaveModpack() {}

private:
    Ui::MainWindow *     ui;
    SearchModsDialog *   searchModsDialog;
    CreateModpackDialog *createModpackDialog;

    Modpack currentModpack;
};
