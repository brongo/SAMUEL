#include "../core/SAMUEL.h"
#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        // Get base path from resource path
        std::string resourcePath = fileName.toStdString();
        auto baseIndex = resourcePath.find("base");
        if (baseIndex == -1)
        {
            fprintf(stderr, "Error: Failed to get game's base path.\n");
        }
        std::string basePath = resourcePath.substr(0, baseIndex + 4);

        // Get export path from argv[0]
        std::string tmpPath = QCoreApplication::applicationFilePath().toStdString();
        std::string exportPath = fs::absolute(tmpPath).replace_filename("exports").string();

        HAYDEN::SAMUEL SAM;
        SAM.Init(basePath);
        SAM.LoadResource(resourcePath);
        SAM.ExportAll(exportPath);
        return;
    }
}

