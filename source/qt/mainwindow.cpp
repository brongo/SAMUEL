#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ThrowFatalError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    return;
}
void MainWindow::ThrowError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    return;
}
int MainWindow::ConfirmExportAll() 
{
    const QString qErrorMessage = "Do you really want to export *everything* in this resource file?";
    const QString qErrorDetail = "This will take some time.";

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    int result = 0;
    result = msgBox.exec();
    return result;
}
void MainWindow::PopulateGUIResourceTable()
{
    // Clear any existing contents
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    // Load resource file data
    HAYDEN::ResourceFile resourceFile = SAM.GetResourceFile();
    for (int i = 0; i < resourceFile.resourceEntries.size(); i++)
    {
        // Temporary, probably? Skip anything that isn't TGA, LWO, MD6 for now.
        if (resourceFile.resourceEntries[i].version != 67 &&
            resourceFile.resourceEntries[i].version != 31 &&
            resourceFile.resourceEntries[i].version != 21)
            continue;

        int row_count = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row_count);

        // Set Resource Name
        std::string resourceName = resourceFile.resourceEntries[i].name;
        QString qResourceName = QString::fromStdString(resourceName);
        QTableWidgetItem *tableResourceName = new QTableWidgetItem(qResourceName);

        // Set Resource Type
        std::string resourceType = resourceFile.resourceEntries[i].type;
        QString qResourceType = QString::fromStdString(resourceType);
        QTableWidgetItem *tableResourceType = new QTableWidgetItem(qResourceType);

        // Set Resource Version
        std::string resourceVersion = std::to_string(resourceFile.resourceEntries[i].version);
        QString qResourceVersion = QString::fromStdString(resourceVersion);
        QTableWidgetItem *tableResourceVersion = new QTableWidgetItem(qResourceVersion);

        // Set Resource Status
        QTableWidgetItem *tableResourceStatus = new QTableWidgetItem("Loaded");

        // Populate Table Row
        ui->tableWidget->setItem(row_count, 0, tableResourceName);
        ui->tableWidget->setItem(row_count, 1, tableResourceType);
        ui->tableWidget->setItem(row_count, 2, tableResourceVersion);
        ui->tableWidget->setItem(row_count, 3, tableResourceStatus);
    }

    QHeaderView* tableHeader = ui->tableWidget->horizontalHeader();
    tableHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    return;
}

void MainWindow::on_btnSettings_clicked()
{
    ThrowError("We do a little trolling.");
    return;
}
void MainWindow::on_btnExportAll_clicked()
{
    if (!_ResourceFileIsLoaded)
    {
        ThrowError("No .resource file is currently loaded.", "Please select a file using the \"Load Resource\" button first.");
        return;
    }
        
    if (ConfirmExportAll() == 0x0400) // OK
        SAM.ExportAll(_ExportPath);
    return;
}
void MainWindow::on_btnExportSelected_clicked()
{
    // Get selected items in QList
    QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();

    // Convert to vector, get text for selected rows
    std::vector<std::vector<std::string>> itemExportRows;
    for (int64 i = 0; i < itemExportQList.size(); i+=4)
    {
        std::vector<std::string> rowText = {
            itemExportQList[i]->text().toStdString(),
            itemExportQList[i+1]->text().toStdString(),
            itemExportQList[i+2]->text().toStdString()
        };
        itemExportRows.push_back(rowText);

        // Update status to "Exported"
        itemExportQList[i+3]->setText("Exported");
    }

    // This vector gets passed to file exporter, so it only exports files from this list.
    SAM.ExportSelected(_ExportPath, itemExportRows);
    return;
}
void MainWindow::on_btnLoadResource_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        _ApplicationPath = QCoreApplication::applicationFilePath().toStdString();
        _ExportPath = fs::absolute(_ApplicationPath).replace_filename("exports").string();
        _ResourcePath = fileName.toStdString();

        // Load Resource Data into SAMUEL
        if (!SAM.Init(_ResourcePath))
        {
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
            return;
        }

        SAM.LoadResource(_ResourcePath);
        PopulateGUIResourceTable();
        _ResourceFileIsLoaded = 1;
    }
}


