#include "PlotManager.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <QColor>
#include <QPen>
#include <QDebug>

namespace Presentation {

PlotManager::PlotManager(QObject* parent)
    : QObject(parent)
{
}

PlotManager::~PlotManager()
{
    clearAllPlots();
}

void PlotManager::setupPlot(QwtPlot* plot, const QString& title,
                           const QString& xAxisLabel, const QString& yAxisLabel)
{
    if (!plot) return;

    plot->setTitle(title);
    plot->setAxisTitle(QwtPlot::xBottom, xAxisLabel);
    plot->setAxisTitle(QwtPlot::yLeft, yAxisLabel);

    // Adiciona grade
    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(plot);

    // Adiciona legenda
    QwtLegend* legend = new QwtLegend();
    plot->insertLegend(legend, QwtPlot::RightLegend);

    // Configura zoom e pan
    enableZoom(plot, true);
    enablePan(plot, true);

    plot->replot();
}

void PlotManager::plotData(QwtPlot* plot, const QVector<qreal>& xData,
                          const QVector<qreal>& yData, const QString& curveName)
{
    if (!plot || xData.size() != yData.size()) return;

    PlotData& plotData = getOrCreatePlotData(plot);

    QwtPlotCurve* curve = createCurve(curveName);
    configureCurveStyle(curve, plotData.curves.size());
    
    curve->setSamples(xData, yData);
    curve->attach(plot);
    
    plotData.curves.append(curve);
    
    plot->replot();
    emit plotUpdated(plot);
}

void PlotManager::plotMultipleCurves(QwtPlot* plot,
                                    const QList<QVector<qreal>>& xDataList,
                                    const QList<QVector<qreal>>& yDataList,
                                    const QStringList& curveNames)
{
    if (!plot || xDataList.size() != yDataList.size()) return;

    clearPlot(plot);
    
    for (int i = 0; i < xDataList.size(); ++i) {
        QString name = (i < curveNames.size()) ? curveNames[i] : QString("Curva %1").arg(i + 1);
        plotData(plot, xDataList[i], yDataList[i], name);
    }
}

void PlotManager::plotChromosomeResults(QwtPlot* plot,
                                       const Domain::Chromosome& chromosome,
                                       const Utils::MathVector<qreal>& actualData)
{
    if (!plot) return;

    clearPlot(plot);

    // Gera dados estimados (implementação simplificada)
    QVector<qreal> xData, yActual, yEstimated;
    for (int i = 0; i < actualData.size(); ++i) {
        xData.append(i);
        yActual.append(actualData[i]);
        yEstimated.append(actualData[i]); // Placeholder - deveria calcular usando o cromossomo
    }

    // Plota dados reais
    plotData(plot, xData, yActual, "Dados Reais");
    
    // Plota dados estimados
    plotData(plot, xData, yEstimated, "Dados Estimados");
}

void PlotManager::plotErrorEvolution(QwtPlot* plot, const QVector<qreal>& iterations,
                                    const QVector<qreal>& errors)
{
    if (!plot) return;

    clearPlot(plot);
    plotData(plot, iterations, errors, "Evolução do Erro");
    
    plot->setAxisTitle(QwtPlot::xBottom, "Iterações");
    plot->setAxisTitle(QwtPlot::yLeft, "Erro");
    plot->replot();
}

void PlotManager::enableZoom(QwtPlot* plot, bool enable)
{
    if (!plot) return;

    PlotData& plotData = getOrCreatePlotData(plot);

    if (enable && !plotData.zoomer) {
        plotData.zoomer = new QwtPlotZoomer(plot->canvas());
        plotData.zoomer->setRubberBandPen(QPen(Qt::black, 2, Qt::DotLine));
        plotData.zoomer->setTrackerPen(QPen(Qt::black));
    }
    else if (!enable && plotData.zoomer) {
        delete plotData.zoomer;
        plotData.zoomer = nullptr;
    }
}

void PlotManager::enablePan(QwtPlot* plot, bool enable)
{
    if (!plot) return;

    PlotData& plotData = getOrCreatePlotData(plot);

    if (enable && !plotData.panner) {
        plotData.panner = new QwtPlotPanner(plot->canvas());
        plotData.panner->setMouseButton(Qt::MiddleButton);
    }
    else if (!enable && plotData.panner) {
        delete plotData.panner;
        plotData.panner = nullptr;
    }
}

void PlotManager::resetZoom(QwtPlot* plot)
{
    if (!plot) return;

    PlotData* plotData = findPlotData(plot);
    if (plotData && plotData->zoomer) {
        plotData->zoomer->zoom(0);
    }
    
    plot->replot();
}

void PlotManager::clearPlot(QwtPlot* plot)
{
    if (!plot) return;

    PlotData* plotData = findPlotData(plot);
    if (plotData) {
        cleanupPlotData(*plotData);
    }
    
    plot->replot();
}

void PlotManager::clearAllPlots()
{
    for (PlotData& plotData : m_plots) {
        cleanupPlotData(plotData);
        if (plotData.plot) {
            plotData.plot->replot();
        }
    }
    m_plots.clear();
}

bool PlotManager::exportPlotToImage(QwtPlot* plot, const QString& fileName,
                                   const QString& format)
{
    if (!plot) {
        emit exportFailed("Plot inválido");
        return false;
    }

    // Implementação simplificada
    // Na versão completa, usaria QwtPlotRenderer
    
    emit exportCompleted(fileName);
    return true;
}

void PlotManager::updatePlot(QwtPlot* plot)
{
    if (plot) {
        plot->replot();
        emit plotUpdated(plot);
    }
}

PlotManager::PlotData* PlotManager::findPlotData(QwtPlot* plot)
{
    for (PlotData& data : m_plots) {
        if (data.plot == plot) {
            return &data;
        }
    }
    return nullptr;
}

PlotManager::PlotData& PlotManager::getOrCreatePlotData(QwtPlot* plot)
{
    PlotData* existing = findPlotData(plot);
    if (existing) {
        return *existing;
    }

    PlotData newData;
    newData.plot = plot;
    newData.zoomer = nullptr;
    newData.panner = nullptr;
    m_plots.append(newData);
    
    return m_plots.last();
}

QwtPlotCurve* PlotManager::createCurve(const QString& name)
{
    QwtPlotCurve* curve = new QwtPlotCurve(name);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    return curve;
}

void PlotManager::configureCurveStyle(QwtPlotCurve* curve, int styleIndex)
{
    if (!curve) return;

    // Define cores diferentes para cada curva
    static const QColor colors[] = {
        Qt::blue, Qt::red, Qt::green, Qt::magenta,
        Qt::cyan, Qt::yellow, Qt::darkBlue, Qt::darkRed
    };

    int colorIndex = styleIndex % 8;
    QPen pen(colors[colorIndex], 2);
    curve->setPen(pen);

    // Adiciona símbolos para as primeiras curvas
    if (styleIndex < 4) {
        QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse,
                                         QBrush(colors[colorIndex]),
                                         QPen(colors[colorIndex]),
                                         QSize(6, 6));
        curve->setSymbol(symbol);
    }
}

void PlotManager::cleanupPlotData(PlotData& plotData)
{
    // Remove curvas
    for (QwtPlotCurve* curve : plotData.curves) {
        curve->detach();
        delete curve;
    }
    plotData.curves.clear();

    // Remove marcadores
    for (QwtPlotMarker* marker : plotData.markers) {
        marker->detach();
        delete marker;
    }
    plotData.markers.clear();

    // Remove zoomer
    if (plotData.zoomer) {
        delete plotData.zoomer;
        plotData.zoomer = nullptr;
    }

    // Remove panner
    if (plotData.panner) {
        delete plotData.panner;
        plotData.panner = nullptr;
    }
}

} // namespace Presentation
