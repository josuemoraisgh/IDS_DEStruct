#include <QProgressBar>
#include <QtCore>
#include <QTimer>
#include "ICarregar.h"
#include "vetor.h"
#include "lev_marq.h"

PSim ICarregar::UL_variaveis;
quint32 ICarregar::UL_count = 0,
        ICarregar::UL_item = 0,
        ICarregar::UL_pos = 0,
        ICarregar::UL_linha = 0;
QList<quint16> ICarregar::UL_cVariaveis;
////////////////////////////////////////////////////////////////////////////
///////////////////////////////Funções//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
bool CompMaiorValores(const QVector<qreal> &cr1, const QVector<qreal> &cr2)
{
    return (cr1.first() < cr2.first());
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////Classe /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
ICarregar::ICarregar(QWidget* parent)
    : DICarregar(parent)
{
    setupUi(this);
}
////////////////////////////////////////////////////////////////////////////
ICarregar::~ICarregar()
{

}
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Carregar()
{
    quint32 i,j,nLinhas;
    qreal valor;
    QVector<quint32> IPonto;
    QFile file(UL_fileName);
    DIC_pb->setRange(0,6);

    //Obtendo as posições de das variaves que serão coletadas
    LVStBar->setText(QObject::trUtf8("Inicializando..."));
    nLinhas = LVSaida->selectionModel()->Rows;
    if(LVSaida->selectionModel()->selectedIndexes().size())
    {
        UL_cVariaveis.insert(0,LVSaida->selectionModel()->selectedIndexes().at(0).row());
        UL_variaveis.nome.append(UL_CabecalhoList.at(UL_cVariaveis.at(0)));
    }
    if(LVEntradas->selectionModel()->selectedIndexes().size())
    {
        for(i=0;i < LVEntradas->selectionModel()->selectedIndexes().size();i++)
        {
            UL_cVariaveis.append(LVEntradas->selectionModel()->selectedIndexes().at(i).row());
            UL_variaveis.nome.append(UL_CabecalhoList.at(LVEntradas->selectionModel()->selectedIndexes().at(i).row()));
        }
    }
    //Tratando dados
    LVStBar->setText(QObject::trUtf8("Tratando dados..."));
    if (file.open(QFile::ReadOnly | QFile::Text))
    {     
        QVector<qreal> valores(UL_cVariaveis.size(),0);
        QTextStream stream(&file);
        stream.readLine();
        do
        {
            j=0;
            for(i=0;i<nLinhas;i++)
            {
                 stream >> valor;
                 if(UL_cVariaveis.at(0)==i) {valores.replace(0,valor);j++;}
                 else {valores.replace(j,valor);j++;}

            }
            UL_variaveis.valores.append(valores);
        }
        while (!stream.atEnd());
        file.close();
    }
/*
    for(i=0;i < UL_cVariaveis.size();i++)
    {
        UL_variaveis.Vmaior.append(0);
        UL_variaveis.Vmenor.append(0);
    }
    DIC_pb->setValue(1);


    //vl = QtConcurrent::blockingFilteredReduced(stream.readAll(),UL_caregaVariaveis,UL_caregaFilterVariaveis,QtConcurrent::SequentialReduce);
    //UL_variaveis.valores = vl.valores;
    DIC_pb->setValue(2);

    //if(!vl.valores.isEmpty())
    //{
        //Capturando dados
        LVStBar->setText(QObject::trUtf8("Capturando dados..."));

        //Ordenando os dados
        LVStBar->setText(QObject::trUtf8("Ordenando os Dados..."));
        qSort(UL_variaveis.valores.begin(),UL_variaveis.valores.end(),CompMaiorValores);
        DIC_pb->setValue(4);

        //IPonto = QtConcurrent::blockingMapped(vl.valores,UL_ipontoVariaveis);

        //Normalizando os Dados
        LVStBar->setText(QObject::trUtf8("Normalizando os Dados..."));
        QtConcurrent::blockingMap(IPonto,UL_normalizaVariaveis);
        DIC_pb->setValue(5);

        //Filtrando os Dados
        LVStBar->setText(QObject::trUtf8("Filtrando os Dados..."));
        //vl.valores = QtConcurrent::blockingFilteredReduced(IPonto,UL_compVariaveis,UL_filtraVariaveis,QtConcurrent::SequentialReduce);
        //UL_variaveis.valores = vl.valores;
        DIC_pb->setValue(6);

        IPonto.clear();
        UL_cVariaveis.clear();
    //}*/
    close();
}
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Caminho()
{
    slot_UL_Indicar(QFileDialog::getOpenFileName(this,tr("Carregar Dados"), QDir::currentPath() , tr("Arquivo de Dados (*.txt)")));
}
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Indicar(QString fileName)
{
    bool isok = false;
    QString line;
    UL_fileName = fileName;

    if (!UL_fileName.isEmpty())
    {
        LECaminho->setText(UL_fileName);
        QFile file(UL_fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream stream(&file);
            line = stream.readLine();
            if (!line.isEmpty())
            {
                UL_CabecalhoList.clear();
                UL_CabecalhoList.append(line.split(QRegExp("(\\s+)")));
                UL_slm = new QStringListModel(UL_CabecalhoList);
                LVEntradas->setSelectionMode(QAbstractItemView::MultiSelection);
                LVEntradas->setModel(UL_slm);
                LVSaida->setModel(UL_slm);
                isok = true;
            }
            file.close();
        }
    }
    if(!isok)
    {
        UL_slm = new QStringListModel();
        LVEntradas->setModel(UL_slm);
        LVSaida->setModel(UL_slm);
    }
}
////////////////////////////////////////////////////////////////////////////
