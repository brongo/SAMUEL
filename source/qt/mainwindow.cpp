#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Public Functions
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnClear->setVisible(false);
    ui->btnSearch->setEnabled(false);
    ui->btnExportAll->setEnabled(false);
    ui->btnExportSelected->setEnabled(false);
}
MainWindow::~MainWindow()
{
    if (_ExportThread != NULL && _ExportThread->isRunning())
        _ExportThread->terminate();
    if (_LoadResourceThread != NULL && _LoadResourceThread->isRunning())
        _LoadResourceThread->terminate();

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

// Private Functions
void MainWindow::DisableGUI()
{
    ui->inputSearch->clear();
    ui->inputSearch->setEnabled(false);
    ui->btnSearch->setEnabled(false);
    ui->btnExportAll->setEnabled(false);
    ui->btnExportSelected->setEnabled(false);
    ui->btnLoadResource->setEnabled(false);
    ui->tableWidget->setEnabled(false);
    return;
}
void MainWindow::EnableGUI()
{
    ui->inputSearch->setEnabled(true);
    ui->btnSearch->setEnabled(true);
    ui->btnExportAll->setEnabled(true);
    ui->btnExportSelected->setEnabled(true);
    ui->btnLoadResource->setEnabled(true);
    ui->tableWidget->setEnabled(true);
    return;
}
void MainWindow::PopulateGUIResourceTable(std::string searchText)
{
    // Must disable sorting or rows won't populate correctly
    ui->tableWidget->setSortingEnabled(false);

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
            resourceFile.resourceEntries[i].version != 21 &&
            resourceFile.resourceEntries[i].version != 0)
            continue;

        // Filter out unsupported "version 0" files.
        if (resourceFile.resourceEntries[i].version == 0 && resourceFile.resourceEntries[i].type != "rs_streamfile")
            continue;

        // Filter out anything we didn't search for
        if (!searchText.empty() && resourceFile.resourceEntries[i].name.find(searchText) == -1)
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

    // Enable sorting again
    ui->tableWidget->setSortingEnabled(true);

    QHeaderView* tableHeader = ui->tableWidget->horizontalHeader();
    tableHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    return;
}
int MainWindow::ConfirmExportAll()
{
    const QString qErrorMessage = "Do you really want to export *everything* in this resource file?";
    const QString qErrorDetail = "For some .resource files this can take 30 minutes or longer, and require 25GB+ of free space.";

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
int MainWindow::ShowLoadStatus()
{
    _LoadStatusBox.setStandardButtons(QMessageBox::Cancel);
    _LoadStatusBox.setText("Loading resource, please wait...");
    int result = 0;
    result = _LoadStatusBox.exec();
    return result;
}
int MainWindow::ShowExportStatus()
{
    _ExportStatusBox.setStandardButtons(QMessageBox::Cancel);
    _ExportStatusBox.setText("Export in progress, please wait...");
    int result = 0;
    result = _ExportStatusBox.exec();
    return result;
}

// Private Slots
void MainWindow::on_btnExportAll_clicked()
{
    if (!_ResourceFileIsLoaded)
    {
        ThrowError("No .resource file is currently loaded.", "Please select a file using the \"Load Resource\" button first.");
        return;
    }
    if (ConfirmExportAll() == 0x0400) // OK
    {
        DisableGUI();
        _ExportThread = QThread::create(&HAYDEN::SAMUEL::ExportAll, &SAM, _ExportPath);
        
        connect(_ExportThread, &QThread::finished, this, [this]() 
        {   
            if (_ExportStatusBox.isVisible())
                _ExportStatusBox.close();

            if (SAM.HasDiskSpaceError() == 1)
                ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());

            if (SAM.HasDiskSpaceError() == 0)
            {
                int rowCount = ui->tableWidget->rowCount();
                for (int64 i = 0; i < rowCount; i++)
                {
                    QTableWidgetItem* tableItem = ui->tableWidget->item(i, 3);
                    tableItem->setText("Exported");
                }
            }
            EnableGUI();
        });

        _ExportThread->start();

        if (ShowExportStatus() == 0x00400000 && _ExportThread->isRunning()) // CANCEL
            _ExportThread->terminate();
    }
    return;
}
void MainWindow::on_btnExportSelected_clicked()
{
    // Get selected items in QList
    QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();

    // Abort if no selection
    if (itemExportQList.size() == 0)
    {
        ThrowError("No items were selected for export.");
        return;
    }

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

        // Load BasePath and PackageMapSpec Data into SAMUEL
        if (!SAM.Init(_ResourcePath))
        {
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
            DisableGUI();
            ui->tableWidget->clearContents();
            ui->tableWidget->setRowCount(0);
            ui->tableWidget->setEnabled(true);
            ui->btnLoadResource->setEnabled(true);
            return;
        }

        DisableGUI();
        _LoadResourceThread = QThread::create(&HAYDEN::SAMUEL::LoadResource, &SAM, _ResourcePath);

        connect(_LoadResourceThread, &QThread::finished, this, [this]()
        {
            if (_LoadStatusBox.isVisible())
                _LoadStatusBox.close();

            if (SAM.HasResourceLoadError() == 1)
            {
                ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
                ui->tableWidget->clearContents();
                ui->tableWidget->setRowCount(0);
            }

            if (SAM.HasResourceLoadError() == 0)
            {
                PopulateGUIResourceTable();
                ui->inputSearch->setEnabled(true);
                ui->btnSearch->setEnabled(true);
                ui->btnExportSelected->setEnabled(true);
                ui->btnExportAll->setEnabled(true);
                _ResourceFileIsLoaded = 1;
            }

            // Enable these even if resource loading failed
            ui->btnLoadResource->setEnabled(true);
            ui->tableWidget->setEnabled(true);
            ui->tableWidget->setSortingEnabled(true);          
        });

        _LoadResourceThread->start();

        if (ShowLoadStatus() == 0x00400000 && _LoadResourceThread->isRunning()) // CANCEL
            _LoadResourceThread->terminate();
    }
}
void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    on_btnExportSelected_clicked();
    return;
}
void MainWindow::on_btnSearch_clicked()
{
    if (ui->inputSearch->text().isEmpty())
        return;

    std::string searchText = ui->inputSearch->text().toStdString();
    PopulateGUIResourceTable(searchText);

    ui->btnClear->setVisible(true);
    return;
}
void MainWindow::on_inputSearch_returnPressed()
{
    on_btnSearch_clicked();
    return;
}
void MainWindow::on_btnClear_clicked()
{
    ui->btnClear->setVisible(false);
    ui->inputSearch->setText("");
    PopulateGUIResourceTable();
    return;
}


