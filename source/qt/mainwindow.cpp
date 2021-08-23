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
    HAYDEN::ResourceFile resourceFile = SAM.GetResourceFile();
    for (int i = 0; i < resourceFile.resourceEntries.size(); i++)
    {
        // Temporary, probably? Skip anything that isn't TGA, LWO, MD6, DECL for now.
        if (resourceFile.resourceEntries[i].version != 67 &&
            resourceFile.resourceEntries[i].version != 31 &&
            resourceFile.resourceEntries[i].version != 21 &&
            resourceFile.resourceEntries[i].version != 0)
            continue;

        // Filter out anything we didn't search for
        if (searchWords.size() > 0)
        {
            _ViewIsFiltered = 1;
            bool matched = 1;

            for (int j = 0; j < searchWords.size(); j++)
                if (resourceFile.resourceEntries[i].name.find(searchWords[j]) == -1)
                    matched = 0;

            if (matched == 0)
                continue;
        }

        // Filter out unsupported .lwo
        if (resourceFile.resourceEntries[i].version == 67)
        {
            if (resourceFile.resourceEntries[i].name.rfind("world_") != -1 && (resourceFile.resourceEntries[i].name.find("maps/game") != -1))
                continue;

            if (resourceFile.resourceEntries[i].name.rfind(".bmodel") != -1)
                continue;
        }

        // Filter out unsupported md6
        if (resourceFile.resourceEntries[i].version == 31)
        {
            if (resourceFile.resourceEntries[i].name.rfind(".abc") != -1)
                continue;
        }

        // Filter out unsupported images
        if (resourceFile.resourceEntries[i].version == 21)
        {
            if (resourceFile.resourceEntries[i].name.rfind("/lightprobes/") != -1)
                continue;
        }

        // Filter out unsupported "version 0" files
        if (resourceFile.resourceEntries[i].version == 0 && resourceFile.resourceEntries[i].type != "rs_streamfile")
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

    QString labelCount = QString::number(ui->tableWidget->rowCount());
    QString labelText = "Found " + labelCount + " files.";
    ui->labelStatus->setText(labelText);

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
void MainWindow::ExportSearchResults()
{
    if (ui->tableWidget->rowCount() == 0)
        return;

    std::vector<std::vector<std::string>> itemExportRows;

    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        std::vector<std::string> rowText = {
            ui->tableWidget->item(i,0)->text().toStdString(),
            ui->tableWidget->item(i,1)->text().toStdString(),
            ui->tableWidget->item(i,2)->text().toStdString()
        };

        itemExportRows.push_back(rowText);
        ui->tableWidget->item(i,3)->setText("Exported");
    }

    _ExportThread = QThread::create(&HAYDEN::SAMUEL::ExportSelected, &SAM, _ExportPath, itemExportRows);

    connect(_ExportThread, &QThread::finished, this, [this]()
    {
        if (_ExportStatusBox.isVisible())
            _ExportStatusBox.close();

        if (SAM.HasDiskSpaceError() == 1)
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());

        QString labelCount = QString::number(ui->tableWidget->rowCount());
        QString labelText = "Exported " + labelCount + " files.";

        ui->labelStatus->setText(labelText);
        EnableGUI();
    });

    _ExportThread->start();

    /*
     * We don't want the dialog box to appear
     * unless the export will take a while. Otherwise it looks broken.
     * For fast exports, the box will flashes on screen and disappear
     * before anyone can read it. Probably needs some adjustment.
    */

    if (itemExportRows.size() >= 40) // Might want to use different number for TGA vs. DECL
    {
        DisableGUI();
        ui->labelStatus->setText("Exporting...");

        if (ShowExportStatus() == 0x00400000 && _ExportThread->isRunning()) // CANCEL
        {
            _ExportThread->terminate();
            ui->labelStatus->setText("Export operation was cancelled.");

            for (int64 i = 0; i < ui->tableWidget->rowCount(); i++)
            {
                QTableWidgetItem* tableItem = ui->tableWidget->item(i, 3);
                tableItem->setText("Loaded");
            }
        }
    }
}


// Private Slots
void MainWindow::on_btnExportAll_clicked()
{
    // User most-likely intends to export search results only.
    if (_ViewIsFiltered == 1)
    {
        ExportSearchResults();
        return;
    }

    // USER CONFIRMED - OK TO EXPORT
    if (ConfirmExportAll() == 0x0400)
    {
        DisableGUI();
        ui->labelStatus->setText("Exporting...");
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
            ui->labelStatus->setText("All files exported successfully.");
        });

        _ExportThread->start();

        // CANCELLED BY USER
        if (ShowExportStatus() == 0x00400000 && _ExportThread->isRunning())
        {
            _ExportThread->terminate();
            ui->labelStatus->setText("Export operation was cancelled.");

            for (int64 i = 0; i <  ui->tableWidget->rowCount(); i++)
            {
                QTableWidgetItem* tableItem = ui->tableWidget->item(i, 3);
                tableItem->setText("Loaded");
            }
        }
    }
    return;
}
void MainWindow::on_btnExportSelected_clicked()
{
    std::vector<std::vector<std::string>> itemExportRows; // list of files to export
    QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();

    if (itemExportQList.size() == 0)
    {
        ThrowError("No items were selected for export.");
        return;
    }

    for (int64 i = 0; i < itemExportQList.size(); i+=4)
    {
        std::vector<std::string> rowText = {
            itemExportQList[i]->text().toStdString(),
            itemExportQList[i+1]->text().toStdString(),
            itemExportQList[i+2]->text().toStdString()
        };
        itemExportRows.push_back(rowText);
        itemExportQList[i+3]->setText("Exported");
    }

    _ExportThread = QThread::create(&HAYDEN::SAMUEL::ExportSelected, &SAM, _ExportPath, itemExportRows);

    connect(_ExportThread, &QThread::finished, this, [this]()
    {
        if (_ExportStatusBox.isVisible())
            _ExportStatusBox.close();

        if (SAM.HasDiskSpaceError() == 1)
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());

        QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();
        QString labelCount = QString::number(itemExportQList.size() / 4);
        QString labelText = "Exported " + labelCount + " files.";

        ui->labelStatus->setText(labelText);
        EnableGUI();
    });

    _ExportThread->start();

    /*
     * We don't want the dialog box to appear
     * unless the export will take a while. Otherwise it looks broken.
     * For fast exports, the box will flashes on screen and disappear
     * before anyone can read it. Probably needs some adjustment.
    */

    if (itemExportRows.size() >= 40) // Might want to use different number for TGA vs. DECL
    {
        DisableGUI();
        ui->labelStatus->setText("Exporting...");

        if (ShowExportStatus() == 0x00400000 && _ExportThread->isRunning()) // CANCEL
        {
            _ExportThread->terminate();
            ui->labelStatus->setText("Export operation was cancelled.");

            for (int64 i = 0; i < ui->tableWidget->rowCount(); i++)
            {
                QTableWidgetItem* tableItem = ui->tableWidget->item(i, 3);
                tableItem->setText("Loaded");
            }
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
        // If it's an AppImage, get path from ARGV0 env variable
        if (getenv("APPIMAGE") != NULL)
            _ApplicationPath = std::string(getenv("ARGV0"));
        else
        #endif
        _ApplicationPath = QCoreApplication::applicationFilePath().toStdString();
        _ExportPath = fs::absolute(_ApplicationPath).replace_filename("exports").string();
        _ResourcePath = fileName.toStdString();

        // Load BasePath and PackageMapSpec Data into SAMUEL
        if (!SAM.Init(_ResourcePath))
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

            if (SAM.HasResourceLoadError() == 1)
            {
                ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
                ResetGUITable();
                ui->labelStatus->setText("Failed to load resource.");
            }

            if (SAM.HasResourceLoadError() == 0)
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
