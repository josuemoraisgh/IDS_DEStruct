#ifndef DATALOADERDIALOG_H
#define DATALOADERDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QList>

// Forward declarations
namespace Ui {
    class DialogCar;
    class DialogMaxMin;
}
class QComboBox;
class QLineEdit;
class QLabel;
class QGroupBox;

namespace Presentation {

/**
 * @brief Dialog de carregamento de dados – réplica fiel do ICarregar original.
 *
 * Responsabilidades:
 * - Permitir ao usuário selecionar um arquivo de dados (.txt)
 * - Mostrar as variáveis (colunas) para seleção de entradas e saídas
 * - Ajuste de tempos inicial/final
 * - Sub-dialog MaxMin para ajustar limites e decimação
 * - Carregar / Concatenar / Cancelar
 *
 * Emite signals que a MainWindow conecta para executar a lógica de carregamento.
 */
class DataLoaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataLoaderDialog(QWidget* parent = nullptr);
    ~DataLoaderDialog() override;

    /** Resetar o status (equivalente a slot_UL_Status(0) no original) */
    void resetStatus();

signals:
    /** Emitido quando o usuário confirmou carregar (com lista de variáveis selecionadas) */
    void dataLoadRequested(const QString& fileName,
                           const QList<qint32>& selectedVariables,
                           qint32 outputCount,
                           double timeInitial, double timeFinal,
                           bool saveToFile);

    /** Emitido quando o nome do arquivo é definido */
    void fileNameSelected(const QString& fileName);

    /** Status genérico para a MainWindow */
    void statusChanged(const QString& message);

public slots:
    void onFinalized();
    void onStopped();

private slots:
    // Slots conectados pela .ui (devem corresponder aos nomes declarados no .ui)
    void slot_UL_Carregar();
    void slot_UL_Indicar(const QString& fileName);
    void slot_UL_Concatenar();
    void slot_UL_Finalizar();
    void slot_UL_Caminho();
    void slot_UL_ModelClicked();

    // Sub-dialog MaxMin
    void onMaxMinComboChanged(int index);
    void onMaxMinFinished(int result);

private:
    void setupMaxMinDialog();

    // UI
    Ui::DialogCar* m_ui;

    // Sub-dialog MaxMin
    QDialog*   m_dialogMaxMin;
    QComboBox* m_dmmComboBox;
    QLineEdit* m_dmmLineEditMax;
    QLineEdit* m_dmmLineEditMin;
    QLineEdit* m_dmmLineEditDECI;
    QLabel*    m_dmmTal10;
    QLabel*    m_dmmTal20;
    QGroupBox* m_dmmGroupBox;

    // Estado interno
    qint32      m_indexVar;
    QString     m_fileName;
    QStringList m_headerList;
    QStringListModel* m_stringListModel;

    // Dados de variáveis (limites, decimação) – armazenados localmente
    QVector<double> m_varMax;
    QVector<double> m_varMin;
    QVector<double> m_decimation;
    QVector<qint32> m_talDecim;
    qint32 m_outputCount;
};

} // namespace Presentation

#endif // DATALOADERDIALOG_H
