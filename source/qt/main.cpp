#include <QApplication>
#include <QCommandLineParser>
#include "../core/SAMUEL.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // temporary instance for dependency check
    HAYDEN::SAMUEL SAM;
    if (!SAM.CheckDependencies())
    {
        w.ThrowFatalError(SAM.GetLastErrorMessage(), SAM.GetLastErrorDetail());
        exit(1);
    }
    SAM.~SAMUEL();

    w.show();
    return a.exec();
}
