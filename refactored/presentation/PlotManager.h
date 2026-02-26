#ifndef REFACTORED_PLOT_MANAGER_H
#define REFACTORED_PLOT_MANAGER_H

#include <QObject>
#include <QVector>
#include "../domain/Chromosome.h"
#include "../utils/MathVector.h"

// Forward declarations para Qwt
class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotZoomer;
class QwtPlotPanner;

namespace Presentation {

/**
 * @brief Gerenciador de gráficos Qwt
 * 
 * Responsabilidade única: Gerenciar todas as operações
 * relacionadas à plotagem de dados usando Qwt
 */
class PlotManager : public QObject
{
    Q_OBJECT

public:
    explicit PlotManager(QObject* parent = nullptr);
    ~PlotManager() override;

    // Configuração de plots
    void setupPlot(QwtPlot* plot, const QString& title, 
                  const QString& xAxisLabel, const QString& yAxisLabel);
    
    // Plotagem de dados
    void plotData(QwtPlot* plot, const QVector<qreal>& xData, 
                 const QVector<qreal>& yData, const QString& curveName);
    
    void plotMultipleCurves(QwtPlot* plot, 
                          const QList<QVector<qreal>>& xDataList,
                          const QList<QVector<qreal>>& yDataList,
                          const QStringList& curveNames);
    
    // Plotagem de resultados do algoritmo
    void plotChromosomeResults(QwtPlot* plot, 
                              const Domain::Chromosome& chromosome,
                              const Utils::MathVector<qreal>& actualData);
    
    void plotErrorEvolution(QwtPlot* plot, const QVector<qreal>& iterations,
                          const QVector<qreal>& errors);
    
    // Controles de zoom e pan
    void enableZoom(QwtPlot* plot, bool enable);
    void enablePan(QwtPlot* plot, bool enable);
    void resetZoom(QwtPlot* plot);
    
    // Limpeza
    void clearPlot(QwtPlot* plot);
    void clearAllPlots();
    
    // Exportação
    bool exportPlotToImage(QwtPlot* plot, const QString& fileName, 
                          const QString& format = "PNG");

public slots:
    void updatePlot(QwtPlot* plot);

signals:
    void plotUpdated(QwtPlot* plot);
    void exportCompleted(const QString& fileName);
    void exportFailed(const QString& error);

private:
    // Estrutura para gerenciar plots
    struct PlotData {
        QwtPlot* plot;
        QList<QwtPlotCurve*> curves;
        QList<QwtPlotMarker*> markers;
        QwtPlotZoomer* zoomer;
        QwtPlotPanner* panner;
    };

    QList<PlotData> m_plots;
    
    // Métodos auxiliares
    PlotData* findPlotData(QwtPlot* plot);
    PlotData& getOrCreatePlotData(QwtPlot* plot);
    QwtPlotCurve* createCurve(const QString& name);
    void configureCurveStyle(QwtPlotCurve* curve, int styleIndex);
    void cleanupPlotData(PlotData& plotData);
};

} // namespace Presentation

#endif // REFACTORED_PLOT_MANAGER_H
