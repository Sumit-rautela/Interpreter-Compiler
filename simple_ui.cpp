#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>

static QString findMyLangExe()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();

    const QStringList candidates = {
        QDir(cwd).filePath("myLang.exe"),
        QDir(appDir).filePath("myLang.exe"),
        QDir(appDir).filePath("../myLang.exe"),
        QDir(appDir).filePath("../../myLang.exe")
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return QDir::cleanPath(path);
        }
    }

    return {};
}

static QString runMyLang(const QString &sourceCode, const QString &mode)
{
    const QString myLangExe = findMyLangExe();
    if (myLangExe.isEmpty()) {
        return "Error: myLang.exe not found. Put this UI next to your project folder or build folder.";
    }

    const QString workDir = QFileInfo(myLangExe).absolutePath();
    const QString inputFile = QDir(workDir).filePath("ui_input.txt");

    if (mode == "--compile") {
        const QString generatedPath = QDir(workDir).filePath("output.cpp");
        if (QFile::exists(generatedPath)) {
            QFile::remove(generatedPath);
        }
    }

    QFile file(inputFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return "Error: could not create temporary input file.";
    }

    QTextStream writer(&file);
    writer << sourceCode;
    file.close();

    QProcess process;
    process.setWorkingDirectory(workDir);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value("PATH");
    const QString appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    const QString exeDirPath = QDir::toNativeSeparators(workDir);

    if (!path.contains(appDirPath, Qt::CaseInsensitive)) {
        path = appDirPath + ";" + path;
    }
    if (!path.contains(exeDirPath, Qt::CaseInsensitive)) {
        path = exeDirPath + ";" + path;
    }
    env.insert("PATH", path);
    process.setProcessEnvironment(env);

    process.start(myLangExe, QStringList() << inputFile << mode);

    if (!process.waitForStarted()) {
        return "Error: failed to start myLang.exe";
    }

    if (!process.waitForFinished(10000)) {
        process.kill();
        return "Error: execution timed out.";
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QString error = QString::fromUtf8(process.readAllStandardError());
    const bool compileSucceeded =
        (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);

    if (mode == "--compile") {
        QStringList filteredLines;
        const QStringList lines = output.split('\n');
        for (const QString &line : lines) {
            if (line.trimmed() == "Generated output.cpp") {
                continue;
            }
            filteredLines << line;
        }
        output = filteredLines.join('\n').trimmed();
    }

    QString result;
    if (!output.trimmed().isEmpty()) {
        result += output;
    }
    if (!error.trimmed().isEmpty()) {
        if (!result.isEmpty()) {
            result += "\n";
        }
        result += error;
    }

    if (mode == "--compile" && compileSucceeded) {
        const QString generatedPath = QDir(workDir).filePath("output.cpp");
        QFile generatedFile(generatedPath);
        if (generatedFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (!result.isEmpty()) {
                result += "\n\n";
            }
            result += QString::fromUtf8(generatedFile.readAll());
        }
    }

    if (result.trimmed().isEmpty()) {
        result = "Done.";
    }

    return result;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("MyLang (Intrepretor - Compiler)");
    window.resize(900, 650);

    auto *root = new QVBoxLayout(&window);

    const QString languageSample =
        "let x = 2.5;\n"
        "let y = 3;\n"
        "let name = \"Sumit\";\n"
        "let letter = 'A';\n"
        "\n"
        "print(name + \" -> \" + letter);\n"
        "print(x + y);\n"
        "\n"
        "if (x < y) {\n"
        "    print(100.25);\n"
        "} else {\n"
        "    print(200);\n"
        "}\n"
        "\n"
        "let i = 0;\n"
        "while (i < 3) {\n"
        "    if (i == 1) {\n"
        "        i = i + 1;\n"
        "        continue;\n"
        "    }\n"
        "    print(i);\n"
        "    i = i + 1;\n"
        "}\n"
        "\n"
        "for (let j = 0; j < 3; j = j + 1) {\n"
        "    if (j == 4) {\n"
        "        break;\n"
        "    }\n"
        "    print(j + 10);\n"
        "}\n";

    auto *codeEditor = new QPlainTextEdit();
    codeEditor->setPlaceholderText("Write your code here...");
    codeEditor->setPlainText(languageSample);

    auto *inputHeader = new QHBoxLayout();
    auto *inputLabel = new QLabel("INPUT");
    inputLabel->setStyleSheet("font-size: 16px; font-weight: 600;");

    auto *interpretBtn = new QPushButton("Interpret");
    auto *compileBtn = new QPushButton("Compile");
    auto *sampleBtn = new QPushButton("Load Sample");

    inputHeader->addWidget(inputLabel);
    inputHeader->addSpacing(10);
    inputHeader->addWidget(interpretBtn);
    inputHeader->addWidget(compileBtn);
    inputHeader->addWidget(sampleBtn);
    inputHeader->addStretch();

    auto *outputLabel = new QLabel("OUTPUT");
    outputLabel->setStyleSheet("font-size: 16px; font-weight: 600;");
    auto *outputView = new QPlainTextEdit();
    outputView->setReadOnly(true);

    auto *mainSplit = new QHBoxLayout();

    auto *leftPane = new QVBoxLayout();
    leftPane->addLayout(inputHeader);
    leftPane->addWidget(codeEditor, 1);

    auto *rightPane = new QVBoxLayout();
    rightPane->addWidget(outputLabel);
    rightPane->addWidget(outputView, 1);

    mainSplit->addLayout(leftPane, 1);
    mainSplit->addLayout(rightPane, 1);

    root->addLayout(mainSplit, 1);

    QObject::connect(sampleBtn, &QPushButton::clicked, [&]() {
        codeEditor->setPlainText(languageSample);
    });

    QObject::connect(interpretBtn, &QPushButton::clicked, [&]() {
        outputView->setPlainText(runMyLang(codeEditor->toPlainText(), "--interpret"));
    });

    QObject::connect(compileBtn, &QPushButton::clicked, [&]() {
        outputView->setPlainText(runMyLang(codeEditor->toPlainText(), "--compile"));
    });

    window.show();
    return app.exec();
}
