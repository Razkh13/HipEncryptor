#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QFontDatabase>
#include <QDebug>
#include <QScreen>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QDateTime>
#include <QPainter>
#include <QGroupBox>

class RoundedLineEdit : public QLineEdit
{
public:
    RoundedLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
        setStyleSheet(
            "QLineEdit {"
            "   background-color: rgba(255,255,255,0.9);"
            "   border: 2px solid #A8E6FF;"
            "   border-radius: 13px;"
            "   padding: 8px 12px;"
            "   font-size: 13px;"
            "   min-width: 280px;"
            "}"
            "QLineEdit:focus {"
            "   border: 2px solid #001020;"
            "   background-color: white;"
            "}"
        );
        setMinimumWidth(280);
        setMaximumHeight(36);
    }
};

class GlacierGradientWidget : public QWidget
{
public:
    GlacierGradientWidget(QWidget* parent = nullptr) : QWidget(parent) {}
    
protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        QLinearGradient gradient(0, 0, width(), height());
        gradient.setColorAt(0, QColor(0xA8, 0xE6, 0xFF));
        gradient.setColorAt(1, QColor(0x00, 0x10, 0x20));
        painter.fillRect(rect(), gradient);
        QWidget::paintEvent(event);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString fontPath = "/home/zaharov/programming/qt_test/NeutralFace.otf";
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    
    QString fontFamily = "Arial";
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            fontFamily = families.first();
        }
    }

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int screenW = screenGeometry.width();
    int screenH = screenGeometry.height();

    int winW = screenW * 65 / 100;
    int winH = screenH * 65 / 100;
    int winX = (screenW - winW) / 2;
    int winY = (screenH - winH) / 2;

    GlacierGradientWidget window;
    window.setGeometry(winX, winY, winW, winH);
    window.setWindowTitle("HipEncryptor");

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(28, 18, 28, 18);

    mainLayout->addSpacing(18);
    
    QLabel* title = new QLabel();
    title->setAlignment(Qt::AlignCenter);
    
    QString htmlText = QString(
        "<html><center>"
        "<span style='font-family: %1; font-size: 42px; font-weight: bold; color: #06294a;'>"
        "<span style='font-size: 60px;'>H</span>"
        "ip"
        "<span style='font-size: 60px;'>E</span>"
        "ncryptor"
        "</span>"
        "</center></html>"
    ).arg(fontFamily);
    
    title->setText(htmlText);
    mainLayout->addWidget(title);

    QLabel* algoLabel = new QLabel("Выберите шифр:");
    QFont labelFont(fontFamily, 17, QFont::Bold);
    algoLabel->setFont(labelFont);
    algoLabel->setAlignment(Qt::AlignCenter);
    algoLabel->setStyleSheet("color: #06294a; margin-bottom: 8px;");
    mainLayout->addWidget(algoLabel);
    mainLayout->addSpacing(10);

    QWidget* radioContainer = new QWidget();
    QHBoxLayout* radioLayout = new QHBoxLayout(radioContainer);
    radioLayout->setAlignment(Qt::AlignCenter);
    radioLayout->setSpacing(45);

    QRadioButton* rbBlowfish = new QRadioButton("Blowfish");
    QRadioButton* rbTwofish = new QRadioButton("Twofish");
    QRadioButton* rbCamellia = new QRadioButton("Camellia");

    QFont radioFont("Arial", 18, QFont::Normal);
    rbBlowfish->setFont(radioFont);
    rbTwofish->setFont(radioFont);
    rbCamellia->setFont(radioFont);

    QString radioStyle = 
        "QRadioButton { "
        "   color: #06294a; "
        "   spacing: 14px; "
        "   padding: 0px; "
        "   margin: 0px; "
        "   outline: none; "
        "}"
        "QRadioButton:hover { "
        "   color: #001020; "
        "}"
        "QRadioButton::indicator { "
        "   width: 20px; "
        "   height: 20px; "
        "}"
        "QRadioButton::indicator::unchecked { "
        "   background-color: #A8E6FF; "
        "   border-radius: 10px; "
        "}"
        "QRadioButton::indicator::checked { "
        "   background-color: #06294a; "
        "   border-radius: 10px; "
        "   border: 2px solid #A8E6FF; "
        "}";

    rbBlowfish->setStyleSheet(radioStyle);
    rbTwofish->setStyleSheet(radioStyle);
    rbCamellia->setStyleSheet(radioStyle);
    rbBlowfish->setChecked(true);

    radioLayout->addWidget(rbBlowfish);
    radioLayout->addWidget(rbTwofish);
    radioLayout->addWidget(rbCamellia);
    mainLayout->addWidget(radioContainer);
    mainLayout->addSpacing(8);

    QGroupBox* encryptGroup = new QGroupBox("🔒 ШИФРОВАНИЕ");
    encryptGroup->setStyleSheet(
        "QGroupBox {"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   border: 2px solid #A8E6FF;"
        "   border-radius: 14px;"
        "   margin-top: 14px;"
        "   padding-top: 14px;"
        "   outline: none; "
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 18px;"
        "   padding: 0 12px;"
        "}"
    );
    QVBoxLayout* encryptLayout = new QVBoxLayout(encryptGroup);
    encryptLayout->setSpacing(10);
    encryptLayout->setContentsMargins(18, 12, 18, 12);

    QHBoxLayout* encryptInputLayout = new QHBoxLayout();
    encryptInputLayout->setSpacing(14);
    QLabel* encryptInputLabel = new QLabel("Файл:");
    encryptInputLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    encryptInputLabel->setFixedWidth(55);
    encryptInputLayout->addWidget(encryptInputLabel);
    RoundedLineEdit* encryptInputEdit = new RoundedLineEdit();
    encryptInputEdit->setPlaceholderText("Выберите файл для шифрования...");
    encryptInputLayout->addWidget(encryptInputEdit);
    QPushButton* encryptInputBtn = new QPushButton("Обзор");
    encryptInputBtn->setFixedSize(90, 32);
    encryptInputBtn->setStyleSheet("QPushButton { background-color: #001020; color: #A8E6FF; border: 1px solid #A8E6FF; border-radius: 9px; padding: 6px 12px; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: #002040; }");
    encryptInputLayout->addWidget(encryptInputBtn);
    encryptLayout->addLayout(encryptInputLayout);
    
    QHBoxLayout* encryptOutputLayout = new QHBoxLayout();
    encryptOutputLayout->setSpacing(14);
    QLabel* encryptOutputLabel = new QLabel("Выход:");
    encryptOutputLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    encryptOutputLabel->setFixedWidth(55);
    encryptOutputLayout->addWidget(encryptOutputLabel);
    RoundedLineEdit* encryptOutputEdit = new RoundedLineEdit();
    encryptOutputEdit->setPlaceholderText("Куда сохранить результат...");
    encryptOutputLayout->addWidget(encryptOutputEdit);
    QPushButton* encryptOutputBtn = new QPushButton("Обзор");
    encryptOutputBtn->setFixedSize(90, 32);
    encryptOutputBtn->setStyleSheet("QPushButton { background-color: #001020; color: #A8E6FF; border: 1px solid #A8E6FF; border-radius: 9px; padding: 6px 12px; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: #002040; }");
    encryptOutputLayout->addWidget(encryptOutputBtn);
    encryptLayout->addLayout(encryptOutputLayout);
    
    mainLayout->addWidget(encryptGroup);

    QGroupBox* decryptGroup = new QGroupBox("🔓 РАСШИФРОВАНИЕ");
    decryptGroup->setStyleSheet(
        "QGroupBox {"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   border: 2px solid #A8E6FF;"
        "   border-radius: 14px;"
        "   margin-top: 14px;"
        "   padding-top: 14px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 18px;"
        "   padding: 0 12px;"
        "}"
    );
    QVBoxLayout* decryptLayout = new QVBoxLayout(decryptGroup);
    decryptLayout->setSpacing(10);
    decryptLayout->setContentsMargins(18, 12, 18, 12);

    QHBoxLayout* decryptInputLayout = new QHBoxLayout();
    decryptInputLayout->setSpacing(14);
    QLabel* decryptInputLabel = new QLabel("Файл:");
    decryptInputLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    decryptInputLabel->setFixedWidth(55);
    decryptInputLayout->addWidget(decryptInputLabel);
    RoundedLineEdit* decryptInputEdit = new RoundedLineEdit();
    decryptInputEdit->setPlaceholderText("Выберите зашифрованный файл...");
    decryptInputLayout->addWidget(decryptInputEdit);
    QPushButton* decryptInputBtn = new QPushButton("Обзор");
    decryptInputBtn->setFixedSize(90, 32);
    decryptInputBtn->setStyleSheet("QPushButton { background-color: #001020; color: #A8E6FF; border: 1px solid #A8E6FF; border-radius: 9px; padding: 6px 12px; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: #002040; }");
    decryptInputLayout->addWidget(decryptInputBtn);
    decryptLayout->addLayout(decryptInputLayout);
    
    QHBoxLayout* decryptOutputLayout = new QHBoxLayout();
    decryptOutputLayout->setSpacing(14);
    QLabel* decryptOutputLabel = new QLabel("Выход:");
    decryptOutputLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    decryptOutputLabel->setFixedWidth(55);
    decryptOutputLayout->addWidget(decryptOutputLabel);
    RoundedLineEdit* decryptOutputEdit = new RoundedLineEdit();
    decryptOutputEdit->setPlaceholderText("Куда сохранить результат...");
    decryptOutputLayout->addWidget(decryptOutputEdit);
    QPushButton* decryptOutputBtn = new QPushButton("Обзор");
    decryptOutputBtn->setFixedSize(90, 32);
    decryptOutputBtn->setStyleSheet("QPushButton { background-color: #001020; color: #A8E6FF; border: 1px solid #A8E6FF; border-radius: 9px; padding: 6px 12px; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: #002040; }");
    decryptOutputLayout->addWidget(decryptOutputBtn);
    decryptLayout->addLayout(decryptOutputLayout);
    
    mainLayout->addWidget(decryptGroup);

    QGroupBox* keyGroup = new QGroupBox("КЛЮЧ");
    keyGroup->setStyleSheet(
        "QGroupBox {"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   border: 2px solid #A8E6FF;"
        "   border-radius: 14px;"
        "   margin-top: 14px;"
        "   padding-top: 14px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 18px;"
        "   padding: 0 12px;"
        "}"
    );
    QVBoxLayout* keyLayout = new QVBoxLayout(keyGroup);
    keyLayout->setSpacing(10);
    keyLayout->setContentsMargins(18, 12, 18, 12);
    
    QHBoxLayout* keyFileLayout = new QHBoxLayout();
    keyFileLayout->setSpacing(14);
    QLabel* keyFileLabel = new QLabel("Файл:");
    keyFileLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    keyFileLabel->setFixedWidth(55);
    keyFileLayout->addWidget(keyFileLabel);
    RoundedLineEdit* keyEdit = new RoundedLineEdit();
    keyEdit->setPlaceholderText("Путь к файлу ключа...");
    keyFileLayout->addWidget(keyEdit);
    QPushButton* keyBtn = new QPushButton("Обзор");
    keyBtn->setFixedSize(90, 32);
    keyBtn->setStyleSheet("QPushButton { background-color: #001020; color: #A8E6FF; border: 1px solid #A8E6FF; border-radius: 9px; padding: 6px 12px; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: #002040; }");
    keyFileLayout->addWidget(keyBtn);
    keyLayout->addLayout(keyFileLayout);
    
    mainLayout->addWidget(keyGroup);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);
    btnLayout->setSpacing(22);
    
    QPushButton* genKeyBtn = new QPushButton("Генерация ключа");
    QPushButton* encryptBtn = new QPushButton("Зашифровать");
    QPushButton* decryptBtn = new QPushButton("Расшифровать");
    
    genKeyBtn->setFixedHeight(40);
    encryptBtn->setFixedHeight(40);
    decryptBtn->setFixedHeight(40);
    
    genKeyBtn->setFixedWidth(165);
    encryptBtn->setFixedWidth(165);
    decryptBtn->setFixedWidth(165);
    
    QString btnStyle = 
        "QPushButton {"
        "   background-color: #001020;"
        "   color: #A8E6FF;"
        "   border: 2px solid #A8E6FF;"
        "   border-radius: 11px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #002040;"
        "   color: white;"
        "}";
    
    genKeyBtn->setStyleSheet(btnStyle);
    encryptBtn->setStyleSheet(btnStyle);
    decryptBtn->setStyleSheet(btnStyle);
    
    btnLayout->addWidget(genKeyBtn);
    btnLayout->addWidget(encryptBtn);
    btnLayout->addWidget(decryptBtn);
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(8);

    QTextEdit* logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(100);
    logEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: rgba(0,16,32,0.8);"
        "   color: #A8E6FF;"
        "   border: 1px solid #A8E6FF;"
        "   border-radius: 11px;"
        "   font-family: monospace;"
        "   font-size: 11px;"
        "   padding: 9px;"
        "}"
    );
    mainLayout->addWidget(logEdit);

    //Обработчик
    QString projectPath = "/home/zaharov/programming/HipEncryptor";
    
    auto appendLog = [&](const QString& text) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        logEdit->append(QString("[%1] %2").arg(timestamp, text));
    };
    
    auto getAlgorithm = [&]() -> QString {
        if (rbBlowfish->isChecked()) return "blowfish";
        if (rbTwofish->isChecked()) return "twofish";
        return "camellia";
    };
    
    QObject::connect(encryptInputBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getOpenFileName(&window, "Выберите файл", projectPath);
        if (!file.isEmpty()) encryptInputEdit->setText(file);
    });
    
    QObject::connect(encryptOutputBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getSaveFileName(&window, "Сохранить как", projectPath);
        if (!file.isEmpty()) encryptOutputEdit->setText(file);
    });
    
    QObject::connect(decryptInputBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getOpenFileName(&window, "Выберите файл", projectPath);
        if (!file.isEmpty()) decryptInputEdit->setText(file);
    });
    
    QObject::connect(decryptOutputBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getSaveFileName(&window, "Сохранить как", projectPath);
        if (!file.isEmpty()) decryptOutputEdit->setText(file);
    });
    
    QObject::connect(keyBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getOpenFileName(&window, "Выберите ключ", projectPath);
        if (!file.isEmpty()) keyEdit->setText(file);
    });
    
    QObject::connect(genKeyBtn, &QPushButton::clicked, [&]() {
        QString keyFile = QFileDialog::getSaveFileName(&window, "Сохранить ключ", projectPath, "*.bin");
        if (keyFile.isEmpty()) return;
        QString cmd = QString("cd %1 && LD_LIBRARY_PATH=. ./cryptum -a %2 -m generate-key -o \"%3\"")
                      .arg(projectPath, getAlgorithm(), keyFile);
        system(cmd.toUtf8().constData());
        appendLog("Ключ создан: " + keyFile);
        keyEdit->setText(keyFile);
    });
    
    QObject::connect(encryptBtn, &QPushButton::clicked, [&]() {
        if (encryptInputEdit->text().isEmpty() || encryptOutputEdit->text().isEmpty() || keyEdit->text().isEmpty()) {
            QMessageBox::warning(&window, "Ошибка", "Заполните поля в секции ШИФРОВАНИЕ и укажите ключ!");
            return;
        }
        QString cmd = QString("cd %1 && LD_LIBRARY_PATH=. ./cryptum -a %2 -m encrypt -i \"%3\" -o \"%4\" -k \"%5\"")
                      .arg(projectPath, getAlgorithm(), encryptInputEdit->text(), encryptOutputEdit->text(), keyEdit->text());
        appendLog("Шифрование...");
        system(cmd.toUtf8().constData());
        appendLog("Готово: " + encryptOutputEdit->text());
    });
    
    QObject::connect(decryptBtn, &QPushButton::clicked, [&]() {
        if (decryptInputEdit->text().isEmpty() || decryptOutputEdit->text().isEmpty() || keyEdit->text().isEmpty()) {
            QMessageBox::warning(&window, "Ошибка", "Заполните поля в секции РАСШИФРОВАНИЕ и укажите ключ!");
            return;
        }
        QString cmd = QString("cd %1 && LD_LIBRARY_PATH=. ./cryptum -a %2 -m decrypt -i \"%3\" -o \"%4\" -k \"%5\"")
                      .arg(projectPath, getAlgorithm(), decryptInputEdit->text(), decryptOutputEdit->text(), keyEdit->text());
        appendLog("Расшифрование...");
        system(cmd.toUtf8().constData());
        appendLog("Готово: " + decryptOutputEdit->text());
    });

    appendLog("HipEncryptor ждет команд!");
    window.show();
    return app.exec();
}