#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Public Functions
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnClear->setVisible(false);
    ui->btnSearch->setEnabled(false);
    ui->btnExportSelected->setEnabled(false);
    ui->radioShowAll->setEnabled(false);
    ui->radioShowDecl->setEnabled(false);
    ui->radioShowEntities->setEnabled(false);
    ui->radioShowImages->setEnabled(false);
    ui->radioShowModels->setEnabled(false);
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
    ui->btnExportSelected->setEnabled(false);
    ui->btnLoadResource->setEnabled(false);
    ui->tableWidget->setEnabled(false);
    ui->radioShowAll->setEnabled(false);
    ui->radioShowDecl->setEnabled(false);
    ui->radioShowEntities->setEnabled(false);
    ui->radioShowImages->setEnabled(false);
    ui->radioShowModels->setEnabled(false);
    return;
}
void MainWindow::EnableGUI()
{
    ui->inputSearch->setEnabled(true);
    ui->btnSearch->setEnabled(true);
    ui->btnExportSelected->setEnabled(true);
    ui->btnLoadResource->setEnabled(true);
    ui->tableWidget->setEnabled(true);
    ui->radioShowAll->setEnabled(true);
    ui->radioShowDecl->setEnabled(true);
    ui->radioShowEntities->setEnabled(true);
    ui->radioShowImages->setEnabled(true);
    ui->radioShowModels->setEnabled(true);
    return;
}
void MainWindow::ResetGUITable()
{
    _ViewIsFiltered = 0;
    ui->labelStatus->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setEnabled(true);
    ui->tableWidget->setSortingEnabled(true);
    return;
}

std::vector<std::string> MainWindow::SplitSearchTerms(std::string inputString)
{
    std::string singleWord;
    std::vector<std::string> searchWords;

    // Convert search string to lowercase
    std::for_each(inputString.begin(), inputString.end(), [](char & c) {
        c = ::tolower(c);
    });

    // Split into words, separated by spaces
    const char* delimiter = " ";
    std::stringstream checkline(inputString);

    while(std::getline(checkline, singleWord, *delimiter))
    {
        if (singleWord != delimiter)
            searchWords.push_back(singleWord);
    }
    return searchWords;
}

void MainWindow::PopulateGUIResourceTable(std::vector<std::string> searchWords)
{
    // Must disable sorting or rows won't populate correctly
    ui->tableWidget->setSortingEnabled(false);

    // Clear any existing contents
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    _ViewIsFiltered = 0;

    // Load resource file data
    std::vector<HAYDEN::ResourceEntry> resourceData = SAM.GetResourceData();
    for (int i = 0; i < resourceData.size(); i++)
    {
        switch (_SearchMode)
        {
            case 0:
                if (resourceData[i].Version != 67 &&
                    resourceData[i].Version != 31 &&
                    resourceData[i].Version != 21 &&
                    resourceData[i].Version != 1 &&
                    resourceData[i].Version != 0)
                    continue;
                break;
            case 1:
                if (resourceData[i].Version != 0)
                    continue;
                break;
            case 2:
                if (resourceData[i].Version != 1)
                    continue;
                break;
            case 3:
                if (resourceData[i].Version != 21)
                    continue;
                break;
            case 4:
                if ((resourceData[i].Version != 31) && (resourceData[i].Version != 67))
                    continue;
                break;
            default:
                break;
        }

        // Filter out anything we didn't search for
        if (searchWords.size() > 0)
        {
            _ViewIsFiltered = 1;
            bool matched = 1;

            for (int j = 0; j < searchWords.size(); j++)
                if (resourceData[i].Name.find(searchWords[j]) == -1)
                    matched = 0;

            if (matched == 0)
                continue;
        }

        // Filter out unsupported md6
        if (resourceData[i].Version == 31)
        {
            if (resourceData[i].Name.rfind(".abc") != -1)
                continue;
        }

        // Filter out unsupported images
        if (resourceData[i].Version == 21)
        {
            if (resourceData[i].Name.rfind("/lightprobes/") != -1)
                continue;
        }

        // Filter out unsupported "version 1" files
        if (resourceData[i].Version == 1 && resourceData[i].Type != "compfile")
            continue;

        // Filter out unsupported "version 0" files
        if (resourceData[i].Version == 0 && resourceData[i].Type != "rs_streamfile")
            continue;

        int row_count = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row_count);

        // Set Resource Name
        std::string resourceName = resourceData[i].Name;
        QString qResourceName = QString::fromStdString(resourceName);
        QTableWidgetItem *tableResourceName = new QTableWidgetItem(qResourceName);

        // Set Resource Type
        std::string resourceType = resourceData[i].Type;
        QString qResourceType = QString::fromStdString(resourceType);
        QTableWidgetItem *tableResourceType = new QTableWidgetItem(qResourceType);

        // Set Resource Version
        std::string resourceVersion = std::to_string(resourceData[i].Version);
        QString qResourceVersion = QString::fromStdString(resourceVersion);
        QTableWidgetItem *tableResourceVersion = new QTableWidgetItem(qResourceVersion);

        // Set Resource Status
        QTableWidgetItem *tableResourceStatus;

        if (resourceData[i].Version == 31)
            tableResourceStatus = new QTableWidgetItem("Experimental");
        else
            tableResourceStatus = new QTableWidgetItem("Loaded");

        // Populate Table Row
        ui->tableWidget->setItem(row_count, 0, tableResourceName);
        ui->tableWidget->setItem(row_count, 1, tableResourceType);
        ui->tableWidget->setItem(row_count, 2, tableResourceVersion);
        ui->tableWidget->setItem(row_count, 3, tableResourceStatus);
    }

    QString labelCount = QString::number(ui->tableWidget->rowCount());
    QString labelText = "Found " + labelCount + " files.";
    ui->labelStatus->setText(labelText);

    // Enable sorting again
    ui->tableWidget->setSortingEnabled(true);

    QHeaderView* tableHeader = ui->tableWidget->horizontalHeader();
    tableHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    return;
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
void MainWindow::on_btnExportSelected_clicked()
{
    // Return if no items are selected for export
    QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();
    if (itemExportQList.size() == 0)
    {
        ThrowError("No items were selected for export.");
        return;
    }

    // Iterate through table, add items to export list
    std::vector<std::vector<std::string>> itemExportRows;
    for (int64_t i = 0; i < itemExportQList.size(); i+=4)
    {
        std::vector<std::string> rowText = {
            itemExportQList[i]->text().toStdString(),
            itemExportQList[i+1]->text().toStdString(),
            itemExportQList[i+2]->text().toStdString()
        };
        itemExportRows.push_back(rowText);
        itemExportQList[i+3]->setText("Exported");
    }

    // Export files in a separate thread
    _ExportThread = QThread::create(&HAYDEN::SAMUEL::ExportFiles, &SAM, _ExportPath, itemExportRows);
    connect(_ExportThread, &QThread::finished, this, [this]()
    {
        if (_ExportStatusBox.isVisible())
            _ExportStatusBox.close();

        QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();
        QString labelCount = QString::number(itemExportQList.size() / 4);
        QString labelText = "Exported " + labelCount + " files.";

        ui->labelStatus->setText(labelText);
        EnableGUI();
    });
    _ExportThread->start();

    // Show status dialog
    if (itemExportRows.size() >= 40)
    {
        DisableGUI();
        ui->labelStatus->setText("Exporting...");
    }

    // Cancelled by user
    if (ShowExportStatus() == 0x00400000 && _ExportThread->isRunning())
    {
        _ExportThread->terminate();
        ui->labelStatus->setText("Export operation was cancelled.");

        for (int64_t i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            QTableWidgetItem* tableItem = ui->tableWidget->item(i, 3);
            tableItem->setText("Loaded");
        }
    }

    return;
}
void MainWindow::on_btnLoadResource_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        #ifdef __linux__
        // If it's an AppImage, get path from OWD and ARGV0 env variables
        if (getenv("APPIMAGE") != NULL)
            _ApplicationPath = fs::path(getenv("OWD")).append(getenv("ARGV0")).string();
        else
        #endif
        _ApplicationPath = QCoreApplication::applicationFilePath().toStdString();
        _ExportPath = fs::absolute(_ApplicationPath).replace_filename("exports").string();
        _ResourcePath = fileName.toStdString();

        // Load BasePath and PackageMapSpec Data into SAMUEL
        if (!SAM.Init(_ResourcePath, GlobalResources))
        {
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
            DisableGUI();
            ResetGUITable();
            ui->labelStatus->setText("Failed to load resource.");
            ui->btnLoadResource->setEnabled(true);
            return;
        }

        DisableGUI();
        _LoadResourceThread = QThread::create(&HAYDEN::SAMUEL::LoadResource, &SAM, _ResourcePath);

        connect(_LoadResourceThread, &QThread::finished, this, [this]()
        {
            if (_LoadStatusBox.isVisible())
                _LoadStatusBox.close();

            if (SAM.HasResourceLoadError())
            {
                ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
                ResetGUITable();
                ui->labelStatus->setText("Failed to load resource.");
            }
            else
            {
                PopulateGUIResourceTable();
                EnableGUI();
                _ResourceFileIsLoaded = 1;
            }

            // Must enable this even if resource loading failed
            ui->btnLoadResource->setEnabled(true);
        });

        ui->labelStatus->setText("Loading resource...");
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
    std::vector<std::string> searchWords = SplitSearchTerms(searchText);

    PopulateGUIResourceTable(searchWords);

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
void MainWindow::on_radioShowAll_toggled(bool checked)
{
    if (checked)
    {
        _SearchMode = 0;
        PopulateGUIResourceTable();
    }
    return;
}
void MainWindow::on_radioShowDecl_toggled(bool checked)
{
    if (checked)
    {
        _SearchMode = 1;
        PopulateGUIResourceTable();
    }
    return;
}
void MainWindow::on_radioShowEntities_toggled(bool checked)
{
    if (checked)
    {
        _SearchMode = 2;
        PopulateGUIResourceTable();
    }
    return;
}
void MainWindow::on_radioShowImages_toggled(bool checked)
{
    if (checked)
    {
        _SearchMode = 3;
        PopulateGUIResourceTable();
    }
    return;
}
void MainWindow::on_radioShowModels_toggled(bool checked)
{
    if (checked)
    {
        _SearchMode = 4;
        PopulateGUIResourceTable();
    }
    return;
}