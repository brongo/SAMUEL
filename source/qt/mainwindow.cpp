#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Public Functions
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
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

MainWindow::~MainWindow() {
    if (m_exportThread != nullptr && m_exportThread->isRunning())
        m_exportThread->terminate();
    if (m_loadResourceThread != nullptr && m_loadResourceThread->isRunning())
        m_loadResourceThread->terminate();

    delete ui;
}

void MainWindow::ThrowFatalError(const std::string& errorMessage, const std::string& errorDetail) {
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
}

void MainWindow::ThrowError(const std::string& errorMessage, const std::string& errorDetail) {
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
}

// Private Functions
void MainWindow::DisableGUI() {
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
}

void MainWindow::EnableGUI() {
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
}

void MainWindow::ResetGUITable() {
    m_viewIsFiltered = false;
    ui->labelStatus->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setEnabled(true);
    ui->tableWidget->setSortingEnabled(true);
}

std::vector<std::string> MainWindow::SplitSearchTerms(std::string inputString) {
    std::string singleWord;
    std::vector<std::string> searchWords;

    // Convert search string to lowercase
    std::for_each(inputString.begin(), inputString.end(), [](char &c) {
        c = ::tolower(c);
    });

    // Split into words, separated by spaces
    const char *delimiter = " ";
    std::stringstream checkline(inputString);

    while (std::getline(checkline, singleWord, *delimiter)) {
        if (singleWord != delimiter)
            searchWords.push_back(singleWord);
    }
    return searchWords;
}

void MainWindow::PopulateGUIResourceTable(const std::vector<std::string>& searchWords) {
    // Must disable sorting or rows won't populate correctly
    ui->tableWidget->setSortingEnabled(false);

    // Clear any existing contents
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    m_viewIsFiltered = false;

    // Load resource file data
    std::vector<HAYDEN::ResourceEntry> resourceData = SAM.resourceManager().fileList();
    for (auto & resource : resourceData) {
        switch (m_searchMode) {
            case 0:
                if (resource.Version != 67 &&
                    resource.Version != 31 &&
                    resource.Version != 21 &&
                    resource.Version != 1 &&
                    resource.Version != 0)
                    continue;
                break;
            case 1:
                if (resource.Version != 0)
                    continue;
                break;
            case 2:
                if (resource.Version != 1)
                    continue;
                break;
            case 3:
                if (resource.Version != 21)
                    continue;
                break;
            case 4:
                if ((resource.Version != 31) && (resource.Version != 67))
                    continue;
                break;
            default:
                break;
        }

        // Filter out anything we didn't search for
        if (!searchWords.empty()) {
            m_viewIsFiltered = true;
            bool matched = true;

            for (const auto & searchWord : searchWords)
                if (resource.Name.find(searchWord) == -1)
                    matched = false;

            if (matched == 0)
                continue;
        }

        // Filter out unsupported .lwo
        if (resource.Version == 67) {
            if (resource.Name.rfind("world_") != -1 && (resource.Name.find("maps/game") != -1))
                continue;

            if (resource.Name.rfind(".bmodel") != -1)
                continue;
        }

        // Filter out unsupported md6
        if (resource.Version == 31) {
            if (resource.Name.rfind(".abc") != -1)
                continue;
        }

        // Filter out unsupported images
        if (resource.Version == 21) {
            if (resource.Name.rfind("/lightprobes/") != -1)
                continue;
        }

        // Filter out unsupported "version 1" files
        if (resource.Version == 1 && resource.Type != "compfile")
            continue;

        // Filter out unsupported "version 0" files
        if (resource.Version == 0 && resource.Type != "rs_streamfile")
            continue;

        int row_count = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row_count);

        // Set Resource Name
        std::string resourceName = resource.Name;
        QString qResourceName = QString::fromStdString(resourceName);
        QTableWidgetItem *tableResourceName = new QTableWidgetItem(qResourceName);

        // Set Resource Type
        std::string resourceType = resource.Type;
        QString qResourceType = QString::fromStdString(resourceType);
        QTableWidgetItem *tableResourceType = new QTableWidgetItem(qResourceType);

        // Set Resource Version
        std::string resourceVersion = std::to_string(resource.Version);
        QString qResourceVersion = QString::fromStdString(resourceVersion);
        QTableWidgetItem *tableResourceVersion = new QTableWidgetItem(qResourceVersion);

        // Set Resource Status
        QTableWidgetItem *tableResourceStatus;

        if (resource.Version == 31)
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

    QHeaderView *tableHeader = ui->tableWidget->horizontalHeader();
    tableHeader->setSectionResizeMode(0, QHeaderView::Stretch);
}

int MainWindow::ShowLoadStatus() {
    m_loadStatusBox.setStandardButtons(QMessageBox::Cancel);
    m_loadStatusBox.setText("Loading resource, please wait...");
    int result = m_loadStatusBox.exec();
    return result;
}

int MainWindow::ShowExportStatus() {
    m_exportStatusBox.setStandardButtons(QMessageBox::Cancel);
    m_exportStatusBox.setText("Export in progress, please wait...");
    int result = m_exportStatusBox.exec();
    return result;
}

// Private Slots
void MainWindow::on_btnExportSelected_clicked() {
    // Return if no items are selected for export
    QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();
    if (itemExportQList.empty()) {
        ThrowError("No items were selected for export.");
        return;
    }

    // Iterate through table, add items to export list
    std::vector<std::vector<std::string>> itemExportRows;
    for (int64_t i = 0; i < itemExportQList.size(); i += 4) {
        std::vector<std::string> rowText = {
                itemExportQList[i]->text().toStdString(),
                itemExportQList[i + 1]->text().toStdString(),
                itemExportQList[i + 2]->text().toStdString()
        };
        itemExportRows.push_back(rowText);
        itemExportQList[i + 3]->setText("Exported");
    }

    // Export files in a separate thread
    m_exportThread = QThread::create(&HAYDEN::SAMUEL::ExportFiles, &SAM, m_exportPath, itemExportRows);
    connect(m_exportThread, &QThread::finished, this, [this]() {
        if (m_exportStatusBox.isVisible())
            m_exportStatusBox.close();

        QList<QTableWidgetItem *> itemExportQList = ui->tableWidget->selectedItems();
        QString labelCount = QString::number(itemExportQList.size() / 4);
        QString labelText = "Exported " + labelCount + " files.";

        ui->labelStatus->setText(labelText);
        EnableGUI();
    });
    m_exportThread->start();

    // Show status dialog
    if (itemExportRows.size() >= 40) {
        DisableGUI();
        ui->labelStatus->setText("Exporting...");
    }

    // Cancelled by user
    if (ShowExportStatus() == 0x00400000 && m_exportThread->isRunning()) {
        m_exportThread->terminate();
        ui->labelStatus->setText("Export operation was cancelled.");

        for (int32_t i = 0; i < ui->tableWidget->rowCount(); i++) {
            QTableWidgetItem *tableItem = ui->tableWidget->item(i, 3);
            tableItem->setText("Loaded");
        }
    }
}

void MainWindow::on_btnLoadResource_clicked() {
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
#ifdef __linux__
        // If it's an AppImage, get path from OWD and ARGV0 env variables
        if (getenv("APPIMAGE") != NULL)
            _ApplicationPath = fs::path(getenv("OWD")).append(getenv("ARGV0")).string();
        else
#endif
        m_applicationPath = QCoreApplication::applicationFilePath().toStdString();
        m_exportPath = fs::absolute(m_applicationPath).replace_filename("exports").string();
        m_resourcePath = fileName.toStdString();

        // Load BasePath and PackageMapSpec Data into SAMUEL
        if (!SAM.Init(m_resourcePath)) {
            ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
            DisableGUI();
            ResetGUITable();
            ui->labelStatus->setText("Failed to load resource.");
            ui->btnLoadResource->setEnabled(true);
            return;
        }

        DisableGUI();
        m_loadResourceThread = QThread::create(&HAYDEN::SAMUEL::LoadResource, &SAM, m_resourcePath);

        connect(m_loadResourceThread, &QThread::finished, this, [this]() {
            if (m_loadStatusBox.isVisible())
                m_loadStatusBox.close();

            if (SAM.hasResourceLoadError()) {
                ThrowError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
                ResetGUITable();
                ui->labelStatus->setText("Failed to load resource.");
            } else {
                PopulateGUIResourceTable();
                EnableGUI();
                m_resourceFileIsLoaded = true;
            }

            // Must enable this even if resource loading failed
            ui->btnLoadResource->setEnabled(true);
        });

        ui->labelStatus->setText("Loading resource...");
        m_loadResourceThread->start();

        if (ShowLoadStatus() == 0x00400000 && m_loadResourceThread->isRunning()) // CANCEL
            m_loadResourceThread->terminate();
    }
}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) {
    on_btnExportSelected_clicked();
}

void MainWindow::on_btnSearch_clicked() {
    if (ui->inputSearch->text().isEmpty())
        return;

    std::string searchText = ui->inputSearch->text().toStdString();
    std::vector<std::string> searchWords = SplitSearchTerms(searchText);

    PopulateGUIResourceTable(searchWords);

    ui->btnClear->setVisible(true);
}

void MainWindow::on_inputSearch_returnPressed() {
    on_btnSearch_clicked();
}

void MainWindow::on_btnClear_clicked() {
    ui->btnClear->setVisible(false);
    ui->inputSearch->setText("");
    PopulateGUIResourceTable();
}

void MainWindow::on_radioShowAll_toggled(bool checked) {
    if (checked) {
        m_searchMode = 0;
        PopulateGUIResourceTable();
    }
}

void MainWindow::on_radioShowDecl_toggled(bool checked) {
    if (checked) {
        m_searchMode = 1;
        PopulateGUIResourceTable();
    }
}

void MainWindow::on_radioShowEntities_toggled(bool checked) {
    if (checked) {
        m_searchMode = 2;
        PopulateGUIResourceTable();
    }
}

void MainWindow::on_radioShowImages_toggled(bool checked) {
    if (checked) {
        m_searchMode = 3;
        PopulateGUIResourceTable();
    }
}

void MainWindow::on_radioShowModels_toggled(bool checked) {
    if (checked) {
        m_searchMode = 4;
        PopulateGUIResourceTable();
    }
}