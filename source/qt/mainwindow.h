#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

#include "../core/SAMUEL.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        void ThrowFatalError(std::string errorMessage, std::string errorDetail = "");
        void ThrowError(std::string errorMessage, std::string errorDetail = "");
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void on_btnLoadResource_clicked();
        void on_btnExportAll_clicked();
        void on_btnExportSelected_clicked();
        void on_btnSettings_clicked();

    private:
        std::string _ApplicationPath;
        std::string _ExportPath;
        std::string _ResourcePath;
        bool _ResourceFileIsLoaded = 0;

        HAYDEN::SAMUEL SAM;
        Ui::MainWindow *ui;

        int  ConfirmExportAll();
        void PopulateGUIResourceTable();
};

#endif // MAINWINDOW_H
