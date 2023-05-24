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
        static void ThrowFatalError(const std::string& errorMessage, const std::string& errorDetail = "");
        static void ThrowError(const std::string& errorMessage, const std::string& errorDetail = "");
        MainWindow(QWidget *parent = nullptr);
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
        QMessageBox m_loadStatusBox;
        QMessageBox m_exportStatusBox;
        QThread* m_loadResourceThread = nullptr;
        QThread* m_exportThread = nullptr;
        std::string m_applicationPath;
        std::string m_exportPath;
        std::string m_resourcePath;
        bool m_resourceFileIsLoaded = false;
        bool m_viewIsFiltered = false;
        int m_searchMode = -1;

        HAYDEN::SAMUEL SAM;
        Ui::MainWindow *ui;

        int  ShowLoadStatus();
        int  ShowExportStatus();
        void DisableGUI();
        void EnableGUI();
        void ResetGUITable();
        void PopulateGUIResourceTable(const std::vector<std::string>& searchWords = std::vector<std::string>());

        // Splits search query by whitespace
        static std::vector<std::string> SplitSearchTerms(std::string inputString);
};