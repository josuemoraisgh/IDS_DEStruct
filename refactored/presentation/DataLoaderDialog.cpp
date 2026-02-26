#include "DataLoaderDialog.h"
#include "ui_icarregar.h"
#include "ui_dialogMaxMin.h"

#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QProgressBar>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QListView>
#include <QStringListModel>
#include <QAbstractItemView>
#include <algorithm>
#include <cmath>

namespace Presentation {

// ============================================================================
// Construtor / Destrutor
// ============================================================================
DataLoaderDialog::DataLoaderDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::DialogCar)
    , m_dialogMaxMin(nullptr)
    , m_dmmComboBox(nullptr)
    , m_dmmLineEditMax(nullptr), m_dmmLineEditMin(nullptr), m_dmmLineEditDECI(nullptr)
    , m_dmmTal10(nullptr), m_dmmTal20(nullptr), m_dmmGroupBox(nullptr)
    , m_indexVar(0)
    , m_stringListModel(nullptr)
    , m_outputCount(0)
{
    m_ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("Carregar Dados"));

    // ---- Sub-dialog MaxMin -------------------------------------------------
    setupMaxMinDialog();

    // Note: Os slots slot_UL_Carregar, slot_UL_Caminho, slot_UL_Indicar,
    // slot_UL_Concatenar, slot_UL_Finalizar e slot_UL_ModelClicked
    // já estão conectados pelas connections definidas no .ui file.
    // As connections do .ui usam os nomes de slot declarados na seção <slots>.
}

DataLoaderDialog::~DataLoaderDialog()
{
    delete m_ui;
}

// ============================================================================
// setupMaxMinDialog
// ============================================================================
void DataLoaderDialog::setupMaxMinDialog()
{
    m_dialogMaxMin = new QDialog(this);
    (new Ui::DialogMaxMin())->setupUi(m_dialogMaxMin);

    m_dmmComboBox     = m_dialogMaxMin->findChild<QComboBox*>("comboBox");
    m_dmmLineEditMax  = m_dialogMaxMin->findChild<QLineEdit*>("lineEditMax");
    m_dmmLineEditMin  = m_dialogMaxMin->findChild<QLineEdit*>("lineEditMin");
    m_dmmLineEditDECI = m_dialogMaxMin->findChild<QLineEdit*>("lineEditDelta");
    m_dmmTal10        = m_dialogMaxMin->findChild<QLabel*>("labelTal10");
    m_dmmTal20        = m_dialogMaxMin->findChild<QLabel*>("labelTal20");
    m_dmmGroupBox     = m_dialogMaxMin->findChild<QGroupBox*>("groupBox");

    if (m_dmmComboBox)
        connect(m_dmmComboBox, QOverload<int>::of(&QComboBox::activated),
                this, &DataLoaderDialog::onMaxMinComboChanged);
    if (m_dialogMaxMin)
        connect(m_dialogMaxMin, &QDialog::finished,
                this, &DataLoaderDialog::onMaxMinFinished);
}

// ============================================================================
// resetStatus – chamado pela MainWindow antes de exec()
// ============================================================================
void DataLoaderDialog::resetStatus()
{
    if (m_ui->LVStBar)
        m_ui->LVStBar->setText(QString::fromUtf8("Aguardando Usuário..."));
    if (m_ui->DIC_pb)
        m_ui->DIC_pb->setValue(0);
}

// ============================================================================
// slot_UL_Caminho – Selecionar arquivo
// ============================================================================
void DataLoaderDialog::slot_UL_Caminho()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        QString::fromUtf8("Carregar Dados"),
        QDir::currentPath(),
        QString::fromUtf8("Arquivo de Dados (*.txt)"));

    if (!path.isEmpty())
        m_ui->LECaminho->setText(path);
}

// ============================================================================
// slot_UL_Indicar – Quando o caminho muda, parsear cabeçalho do arquivo
// ============================================================================
void DataLoaderDialog::slot_UL_Indicar(const QString& fileName)
{
    bool isOk = false;

    if (!fileName.isEmpty()) {
        m_fileName = fileName;
        emit fileNameSelected(fileName);

        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            QString line = stream.readLine();

            if (!line.isNull()) {
                m_headerList.clear();
                QStringList parts = line.split(QRegularExpression("(\\s+)"));
                if (!parts.isEmpty() && parts.first().isEmpty())
                    parts.removeFirst();
                m_headerList.append(parts);

                m_stringListModel = new QStringListModel(m_headerList, this);
                m_ui->LVEntradas->setSelectionMode(QAbstractItemView::MultiSelection);
                m_ui->LVSaida->setSelectionMode(QAbstractItemView::MultiSelection);
                m_ui->LVEntradas->setModel(m_stringListModel);
                m_ui->LVSaida->setModel(m_stringListModel);
                isOk = true;
            }

            // Ler tempo inicial (segunda linha, primeiro valor)
            stream.setRealNumberNotation(QTextStream::ScientificNotation);
            line = stream.readLine();
            if (!line.isNull()) {
                m_ui->LETIni->setText(
                    line.split(QString(QChar(32)), Qt::SkipEmptyParts).at(0));
                m_ui->CBTIni->setCurrentIndex(0);
            }

            // Ler tempo final (últimas linhas do arquivo)
            quint64 tam = file.size();
            bool found = false;
            do {
                tam -= 50;
                found = stream.seek(tam);
                stream.readLine();
                line = stream.readLine();
            } while (stream.atEnd() || line.size() <= 7 || !found);

            if (!line.isNull() && line.size() > 7) {
                bool confere = false;
                line.split(QString(QChar(32)), Qt::SkipEmptyParts).at(0).toFloat(&confere);
                if (confere)
                    m_ui->LETFim->setText(
                        line.split(QString(QChar(32)), Qt::SkipEmptyParts).at(0));
            }
            // continuar até o final
            do {
                line = stream.readLine();
                if (!line.isNull() && line.size() > 7) {
                    bool confere = false;
                    line.split(QString(QChar(32)), Qt::SkipEmptyParts).at(0).toFloat(&confere);
                    if (confere)
                        m_ui->LETFim->setText(
                            line.split(QString(QChar(32)), Qt::SkipEmptyParts).at(0));
                }
            } while (!stream.atEnd());

            file.close();
            m_ui->CBTFim->setCurrentIndex(0);
        }
    }

    if (!isOk) {
        m_stringListModel = new QStringListModel(this);
        m_ui->LVEntradas->setModel(m_stringListModel);
        m_ui->LVSaida->setModel(m_stringListModel);
    }
}

// ============================================================================
// slot_UL_ModelClicked – Habilitar botões quando há seleção
// ============================================================================
void DataLoaderDialog::slot_UL_ModelClicked()
{
    bool hasEntradas = m_ui->LVEntradas->selectionModel() &&
                       !m_ui->LVEntradas->selectionModel()->selectedIndexes().isEmpty();
    bool hasSaida    = m_ui->LVSaida->selectionModel() &&
                       !m_ui->LVSaida->selectionModel()->selectedIndexes().isEmpty();

    m_ui->BCarregar->setEnabled(hasEntradas && hasSaida);
    m_ui->BConcatenar->setEnabled(hasEntradas && hasSaida);
}

// ============================================================================
// slot_UL_Carregar – Botão "Carregar"
// ============================================================================
void DataLoaderDialog::slot_UL_Carregar()
{
    if (m_ui->BCarregar->text() == "Parar") {
        m_ui->BCarregar->setText("Continuar");
        emit statusChanged("Parado pelo usuário");
        return;
    }

    if (m_ui->BCarregar->text() == "Continuar") {
        m_ui->BCarregar->setText("Parar");
        emit statusChanged("Continuando...");
        return;
    }

    // Inicializar carregamento
    QList<qint32> selectedVars;
    m_ui->BCarregar->setText("Parar");
    m_ui->BCancel->setText("Finalizar");
    m_ui->DIC_pb->setRange(0, 5);

    // Coletar entradas selecionadas
    auto entradaSel = m_ui->LVEntradas->selectionModel()->selectedIndexes();
    for (int i = 0; i < entradaSel.size(); ++i)
        selectedVars.append(entradaSel.at(i).row());
    std::sort(selectedVars.begin(), selectedVars.end());

    // Coletar saídas selecionadas (inseridas no início)
    auto saidaSel = m_ui->LVSaida->selectionModel()->selectedIndexes();
    m_outputCount = saidaSel.size();
    for (int i = 0; i < saidaSel.size(); ++i)
        selectedVars.insert(i, saidaSel.at(i).row());

    double tIni = m_ui->LETIni->text().toDouble() *
                  std::pow(10.0, m_ui->CBTIni->currentIndex() * (-3));
    double tFim = m_ui->LETFim->text().toDouble() *
                  std::pow(10.0, m_ui->CBTFim->currentIndex() * (-3));
    bool saveToFile = m_ui->CBSalvar->isChecked();

    // Passos de status (réplica do original)
    m_ui->LVStBar->setText(QString::fromUtf8("Carregando arquivo de banco de dados..."));
    m_ui->DIC_pb->setValue(1);
    m_ui->LVStBar->repaint();

    emit dataLoadRequested(m_fileName, selectedVars, m_outputCount,
                           tIni, tFim, saveToFile);
}

// ============================================================================
// slot_UL_Concatenar
// ============================================================================
void DataLoaderDialog::slot_UL_Concatenar()
{
    // Mesmo fluxo do Carregar, mas sinaliza concatenação
    slot_UL_Carregar();
}

// ============================================================================
// slot_UL_Finalizar – Quando o dialog é fechado ou finalizado
// ============================================================================
void DataLoaderDialog::slot_UL_Finalizar()
{
    m_ui->BCarregar->setText("Carregar");
    m_ui->BCancel->setText("Fechar");
}

// ============================================================================
// Slots públicos (chamados pela MainWindow quando o algoritmo finaliza/para)
// ============================================================================
void DataLoaderDialog::onFinalized()
{
    if (isActiveWindow())
        m_ui->BCarregar->setEnabled(true);
}

void DataLoaderDialog::onStopped()
{
    if (isActiveWindow())
        m_ui->BCarregar->setEnabled(true);
}

// ============================================================================
// Sub-dialog MaxMin slots
// ============================================================================
void DataLoaderDialog::onMaxMinComboChanged(int indexVar)
{
    // Salvar valores atuais antes de mudar
    if (m_indexVar < m_varMax.size()) {
        m_varMax[m_indexVar]  = m_dmmLineEditMax->text().toDouble();
        m_varMin[m_indexVar]  = m_dmmLineEditMin->text().toDouble();
    }
    if (m_indexVar < m_outputCount && m_indexVar < m_decimation.size())
        m_decimation[m_indexVar] = m_dmmLineEditDECI->text().toDouble();

    m_indexVar = indexVar;

    // Mostrar valores da variável selecionada
    if (indexVar < m_varMax.size()) {
        m_dmmLineEditMax->setText(QString::number(m_varMax.at(indexVar)));
        m_dmmLineEditMin->setText(QString::number(m_varMin.at(indexVar)));
    }
    if (indexVar < m_outputCount && indexVar < m_decimation.size()) {
        if (m_dmmGroupBox) m_dmmGroupBox->setEnabled(true);
        m_dmmLineEditDECI->setText(QString::number(m_decimation.at(indexVar)));
        if (indexVar < m_talDecim.size()) {
            m_dmmTal10->setText(" < " + QString::number(static_cast<double>(m_talDecim.at(indexVar)) / 10.0));
            m_dmmTal20->setText(QString::number(static_cast<double>(m_talDecim.at(indexVar)) / 20.0) + " < ");
        }
    } else {
        if (m_dmmGroupBox) m_dmmGroupBox->setDisabled(true);
    }
}

void DataLoaderDialog::onMaxMinFinished(int /*result*/)
{
    // Salvar última seleção
    if (m_indexVar < m_varMax.size()) {
        m_varMax[m_indexVar]  = m_dmmLineEditMax->text().toDouble();
        m_varMin[m_indexVar]  = m_dmmLineEditMin->text().toDouble();
    }
    if (m_indexVar < m_outputCount && m_indexVar < m_decimation.size())
        m_decimation[m_indexVar] = m_dmmLineEditDECI->text().toDouble();

    m_indexVar = 0;
    emit statusChanged(QString::fromUtf8("Normalização concluída"));
}

} // namespace Presentation
