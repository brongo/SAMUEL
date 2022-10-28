#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QTableWidgetItem>

#include "../core/SAMUEL.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        HAYDEN::GLOBAL_RESOURCES GlobalResources;
        void ThrowFatalError(std::string errorMessage, std::string errorDetail = "");
        void ThrowError(std::string errorMessage, std::string errorDetail = "");
        MainWindow(QWidget *parent = NULL);
        ~MainWindow();

    private slots:
        void on_btnLoadResource_clicked();
        void on_btnExportSelected_clicked();
        void on_btnSearch_clicked();
        void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
        void on_btnClear_clicked();
        void on_inputSearch_returnPressed();
        void on_radioShowAll_toggled(bool checked);
        void on_radioShowDecl_toggled(bool checked);
        void on_radioShowEntities_toggled(bool checked);
        void on_radioShowImages_toggled(bool checked);
        void on_radioShowModels_toggled(bool checked);

    private:
        QMessageBox _LoadStatusBox;
        QMessageBox _ExportStatusBox;
        QThread* _LoadResourceThread = NULL;
        QThread* _ExportThread = NULL;
        std::string _ApplicationPath;
        std::string _ExportPath;
        std::string _ResourcePath;
        bool _ResourceFileIsLoaded = 0;
        bool _ViewIsFiltered = 0;
        int _SearchMode = 0;

        HAYDEN::SAMUEL SAM;
        Ui::MainWindow *ui;

        int  ShowLoadStatus();
        int  ShowExportStatus();
        void DisableGUI();
        void EnableGUI();
        void ResetGUITable();
        void PopulateGUIResourceTable(std::vector<std::string> searchWords = std::vector<std::string>());

        // Splits search query by whitespace
        std::vector<std::string> SplitSearchTerms(std::string inputString);
};