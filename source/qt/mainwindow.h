#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>

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

    private:
        QMessageBox _LoadStatusBox;
        QMessageBox _ExportStatusBox;
        QThread* _LoadResourceThread = NULL;
        QThread* _ExportThread = NULL;
        std::string _ApplicationPath;
        std::string _ExportPath;
        std::string _ResourcePath;
        bool _ResourceFileIsLoaded = 0;

        HAYDEN::SAMUEL SAM;
        Ui::MainWindow *ui;

        int  ConfirmExportAll();
        int  ShowLoadStatus();
        int  ShowExportStatus();
        void PopulateGUIResourceTable();
        void ExportInThread(HAYDEN::SAMUEL& SAM, const std::string exportPath);

};

#endif // MAINWINDOW_H
