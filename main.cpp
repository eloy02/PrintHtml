/*
 * MIT License
 *
 * Copyright (C) 2017 Kendall Bennett
 * Copyright (C) 2017 AMain.com, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "printhtml.h"
#include "globals.h"

/*
 * Main program entry point
 *
 * PARAMETERS:
 * argc - Number of arguments
 * argv - Array of command line arguments
 */
int main(
    int argc,
    char *argv[])
{
    // Start the application. Must be a Windows app in order to use Qt WebKit
    QApplication app(argc, argv);

    // Parse the command line
    QString printer = "Default";
    double leftMargin = 0.5;
    double topMargin = 0.5;
    double rightMargin = 0.5;
    double bottomMargin = 0.5;
    QString paper = "A4";
    QString orientation = "portrait";
    int pageFrom = 0;
    int pageTo = 0;
    QStringList urls;
    bool testMode = false;
    if (argc < 2) {
        QString usage = "Usage: PrintHtml [-test] [-p printer] [-l left] [-t top] [-r right] [-b bottom] [-a paper] [-o orientation] [-pagefrom number] [-pageto number] <url> [url2]\n\n";
        usage += "-test                  \t - Don't print, just show what would have printed.\n \n";
        usage += "-p printer             \t - Printer to print to. Use 'Default' for default printer.\n \n";
        usage += "-l left                \t - Optional left margin for page. (Default 0.5)\n \n";
        usage += "-t top                 \t - Optional top margin for page. (Default 0.5)\n \n";
        usage += "-r right               \t - Optional right margin for page. (Default 0.5)\n \n";
        usage += "-b bottom              \t - Optional bottom margin for page. (Default 0.5)\n \n";
        usage += "-a [A4|A5|Letter]      \t - Optional paper type. (Default A4) \n \n";
        usage += "-o [Portrait|Landscape]\t - Optional orientation type. (Default Portrait)\n \n";
        usage += "-pagefrom number       \t - Optional. Use for setting up the range of pages for printing. Corresponds to the first page in the page range for printing. (Must be used with \"-pageto\" parameter)\n \n";
        usage += "-pageto number         \t - Optional. Use for setting up the range of pages for printing. Corresponds to the last page in the page range for printing. (Must be used with \"-pagefrom\" parameter)\n \n";
        usage += "url                    \t - Defines the list of URLs to print, one after the other.\n \n \n";
        usage += "Note: Pages in a document are numbered according to the convention that the first page is page 1. However, if from and to are both set to 0, the whole document will be printed.";

        QMessageBox msgBox;
        msgBox.setWindowTitle("PrintHtml Usage");
        msgBox.setText(usage);
        msgBox.exec();
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];
        if (arg == "-p")
            printer = argv[++i];
        else if (arg == "-test")
            testMode = true;
        else if (arg == "-l")
            leftMargin = atof(argv[++i]);
        else if (arg == "-t")
            topMargin = atof(argv[++i]);
        else if (arg == "-r")
            rightMargin = atof(argv[++i]);
        else if (arg == "-b")
            bottomMargin = atof(argv[++i]);
        else if (arg == "-a")
            paper = argv[++i];
        else if (arg == "-o")
            orientation = argv[++i];
        else if (arg.toLower() == "-pagefrom")
            pageFrom = atoi(argv[++i]);
        else if (arg.toLower() == "-pageto")
            pageTo = atoi(argv[++i]);
        else
            urls << arg;
    }

    // Find the application directory and store it in our global variable. Note
    // that on MacOS we need to go back three directories to get to the actual
    // directory that contains the application file (which is really a directory
    // on MacOS). Also note that when developing, the redist files are back
    // one directory from the application source directory, in the redist folder.
    // Once the apps are deployed however, the files are all in the same directory.
    // And on Windows, the build files are under the debug directory, so we need
    // to look for them back one directory from the build directory. Confused yet!?
    QString dataPath = qApp->applicationDirPath();

    // Now set the default SSL socket certificates for this program to our own CA bundle
    QFile* caBundle = new QFile(dataPath + "/ca-bundle.crt");
    if (caBundle->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QSslSocket::setDefaultCaCertificates(QSslCertificate::fromDevice(caBundle));
        caBundle->close();
        delete caBundle;
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Fatal Error");
        msgBox.setText("Cannot find SSL certificates bundle!");
        msgBox.exec();
        return -1;
    }

    // Make sure we can find plugins relative to the current directory
    QStringList paths(app.libraryPaths());
    paths.prepend(dataPath + "/plugins");
    app.setLibraryPaths(paths);

    // Create the HTML printer class
    PrintHtml printHtml(testMode, urls, printer, leftMargin, topMargin, rightMargin, bottomMargin, paper, orientation, pageFrom, pageTo);

    // Connect up the signals
    QObject::connect(&printHtml, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), &printHtml, SLOT(aboutToQuitApp()));

    // This code will start the messaging engine in QT and in
    // 10ms it will start the execution in the PrintHtml.run routine;
    QTimer::singleShot(10, &printHtml, SLOT(run()));
    return app.exec();
}
