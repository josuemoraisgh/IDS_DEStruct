#include <QtGui>
#include <xtipodados.h>
#include <QXmlStreamReader>
#include "xmlreaderwriter.h"


XmlReaderWriter::XmlReaderWriter(Config *Ajts)
{
    XWR_Ajts  = Ajts;
    xmlWriter.setAutoFormatting(true);
}

bool XmlReaderWriter::writeFile(QIODevice *device)
{
    qint32 i=0, j=0, k=0, l=0, m=0;
    QString str,strTermo,strCoefic,strExpoente;
    xmlWriter.setDevice(device);

    xmlWriter.writeStartDocument();
    xmlWriter.writeDTD("<!DOCTYPE xbel>");
    xmlWriter.writeStartElement("xbel");
    xmlWriter.writeAttribute("version", "1.0");

    if(XWR_Ajts!=NULL)
    {
        xmlWriter.writeStartElement("Ajustes");
        xmlWriter.writeAttribute("numeroCiclos",QString::number(XWR_Ajts->numeroCiclos));
        xmlWriter.writeAttribute("jnrr",QString::number(XWR_Ajts->jnrr));
        xmlWriter.writeAttribute("serr",QString::number(XWR_Ajts->serr));
        xmlWriter.writeAttribute("isExpoReal",QString::number(XWR_Ajts->isTipoExpo));
        xmlWriter.writeAttribute("isRacional",QString::number(XWR_Ajts->isRacional));

        xmlWriter.writeAttribute("isIniciaEnabled", QString::number(XWR_Ajts->isIniciaEnabled));
        xmlWriter.writeAttribute("salvarAutomati", QString::number(XWR_Ajts->salvarAutomati));        

        str.clear();
        for(i=0;i<XWR_Ajts->decimacao.size();i++) str += QString::number(XWR_Ajts->decimacao.at(i))+" ";
        xmlWriter.writeAttribute("decimacao",str);
        str.clear();
        for(i=0;i<XWR_Ajts->talDecim.size();i++) str += QString::number(XWR_Ajts->talDecim.at(i))+" ";
        xmlWriter.writeAttribute("talDecim",str);

        xmlWriter.writeAttribute("iteracoes", QString::number(XWR_Ajts->iteracoes));
        xmlWriter.writeAttribute("isPararContinuarEnabled", QString::number(XWR_Ajts->isPararContinuarEnabled));
        xmlWriter.writeAttribute("qdadeCrSalvos", QString::number(XWR_Ajts->qdadeCrSalvos));
        xmlWriter.writeAttribute("salvDadosCarre", QString::number(XWR_Ajts->salvDadosCarre));
        xmlWriter.writeAttribute("nomeArqConfTxT", XWR_Ajts->nomeArqConfTxT);
        xmlWriter.writeEndElement();

    //XWR_Ajts->Dados

        xmlWriter.writeStartElement("PSim");
        xmlWriter.writeAttribute("qtSaidas", QString::number(XWR_Ajts->Dados.variaveis.qtSaidas));
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.nome.size();i++) str +=  XWR_Ajts->Dados.variaveis.nome.at(i)+" ";
        xmlWriter.writeAttribute("nome",str);
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.Vmaior.size();i++) str +=  QString::number(XWR_Ajts->Dados.variaveis.Vmaior.at(i))+" ";
        xmlWriter.writeAttribute("Vmaior",str);
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.Vmenor.size();i++) str +=  QString::number(XWR_Ajts->Dados.variaveis.Vmenor.at(i))+" ";
        xmlWriter.writeAttribute("Vmenor",str);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("valores");
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.valores.size();i++)
            str += QString::number(XWR_Ajts->Dados.variaveis.valores.at(i))+" ";
        xmlWriter.writeAttribute("valores",str);
        xmlWriter.writeAttribute("linha",QString::number(XWR_Ajts->Dados.variaveis.valores.numLinhas()));
        xmlWriter.writeAttribute("coluna",QString::number(XWR_Ajts->Dados.variaveis.valores.numColunas()));
        xmlWriter.writeAttribute("iElitismo", QString::number(XWR_Ajts->Dados.iElitismo));
        xmlWriter.writeAttribute("qtdadeVarAnte", QString::number(XWR_Ajts->Dados.qtdadeVarAnte));
        //xmlWriter.writeAttribute("timeAmost", QString::number(XWR_Ajts->Dados.timeAmost));
        xmlWriter.writeAttribute("timeInicial", QString::number(XWR_Ajts->Dados.timeInicial));
        xmlWriter.writeAttribute("timeFinal", QString::number(XWR_Ajts->Dados.timeFinal));
        xmlWriter.writeAttribute("isElitismo", QString::number(XWR_Ajts->Dados.isElitismo));
        xmlWriter.writeEndElement();

    //XWR_Ajts->Pop
        const qint32 saidasCount = qMin<qint32>(XWR_Ajts->Dados.variaveis.qtSaidas, XWR_Ajts->Pop.size());
        for(j=0;j<saidasCount;j++)
        {
            xmlWriter.writeStartElement("Saida"+QString::number(j));
            for(k=0;k<XWR_Ajts->Pop.at(j).size();k++)
            {
                xmlWriter.writeStartElement("Cromossomo"+QString::number(k));
                xmlWriter.writeAttribute("aptidao", QString::number(XWR_Ajts->Pop.at(j).at(k).aptidao));
                xmlWriter.writeAttribute("erro", QString::number(XWR_Ajts->Pop.at(j).at(k).erro));
                xmlWriter.writeAttribute("idSaida", QString::number(XWR_Ajts->Pop.at(j).at(k).idSaida));
                xmlWriter.writeAttribute("maiorAtraso", QString::number(XWR_Ajts->Pop.at(j).at(k).maiorAtraso));
                strTermo.clear();strCoefic.clear();strExpoente.clear();
                const qint32 regressCount = qMin<qint32>(XWR_Ajts->Pop.at(j).at(k).regress.size(),
                                                         qMin<qint32>(XWR_Ajts->Pop.at(j).at(k).err.size(),
                                                                      XWR_Ajts->Pop.at(j).at(k).vlrsCoefic.size()));
                for(l=0;l<regressCount;l++)
                {
                    xmlWriter.writeStartElement("Regress"+QString::number(l));
                    xmlWriter.writeAttribute("Err", QString::number(XWR_Ajts->Pop.at(j).at(k).err.at(l)));
                    xmlWriter.writeAttribute("Coefic", QString::number(XWR_Ajts->Pop.at(j).at(k).vlrsCoefic.at(l)));
                    for(m=0;m<XWR_Ajts->Pop.at(j).at(k).regress.at(l).size();m++)
                    {
                        xmlWriter.writeStartElement("Termo"+QString::number(m));
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.var);
                        xmlWriter.writeAttribute("var",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.atraso);
                        xmlWriter.writeAttribute("atraso",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.reg);
                        xmlWriter.writeAttribute("reg",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.nd);
                        xmlWriter.writeAttribute("nd",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).expoente);
                        xmlWriter.writeAttribute("expoente",strTermo);
                        xmlWriter.writeEndElement();
                    }
                    xmlWriter.writeEndElement();                    
                }
                xmlWriter.writeEndElement();
            }
            xmlWriter.writeEndElement();
        }
    }
    xmlWriter.writeEndDocument();
    return true;
}

bool XmlReaderWriter::readFile(QIODevice *device)
{
    if(XWR_Ajts == NULL) return false;

    xmlRead.clear();
    xmlRead.setDevice(device);

    const auto splitTokens = [](const QString &text) -> QStringList {
        return text.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    };

    XWR_Ajts->decimacao.clear();
    XWR_Ajts->talDecim.clear();
    XWR_Ajts->Dados.variaveis.nome.clear();
    XWR_Ajts->Dados.variaveis.Vmaior.clear();
    XWR_Ajts->Dados.variaveis.Vmenor.clear();
    XWR_Ajts->Dados.variaveis.valores.clear();
    XWR_Ajts->Pop.clear();

    if (!xmlRead.readNextStartElement()) return false;

    if (xmlRead.name() != "xbel" || xmlRead.attributes().value("version") != "1.0")
    {
        xmlRead.raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
        return false;
    }

    while (xmlRead.readNextStartElement())
    {
        const QString nodeName = xmlRead.name().toString();
        if (nodeName == "Ajustes")
        {
            XWR_Ajts->numeroCiclos = xmlRead.attributes().value("numeroCiclos").toString().toInt();
            XWR_Ajts->jnrr = xmlRead.attributes().value("jnrr").toString().toDouble();
            XWR_Ajts->serr = xmlRead.attributes().value("serr").toString().toDouble();
            XWR_Ajts->isTipoExpo = xmlRead.attributes().value("isExpoReal").toString().toInt();
            XWR_Ajts->isRacional = xmlRead.attributes().value("isRacional").toString().toInt();
            XWR_Ajts->isIniciaEnabled = xmlRead.attributes().value("isIniciaEnabled").toString().toInt();
            XWR_Ajts->salvarAutomati = xmlRead.attributes().value("salvarAutomati").toString().toInt();
            const QStringList decimacao = splitTokens(xmlRead.attributes().value("decimacao").toString());
            for(int i=0; i<decimacao.size(); i++) XWR_Ajts->decimacao.push_back(decimacao.at(i).toInt());
            const QStringList talDecim = splitTokens(xmlRead.attributes().value("talDecim").toString());
            for(int i=0; i<talDecim.size(); i++) XWR_Ajts->talDecim.push_back(talDecim.at(i).toInt());
            XWR_Ajts->iteracoes = xmlRead.attributes().value("iteracoes").toString().toLongLong();
            XWR_Ajts->isPararContinuarEnabled = xmlRead.attributes().value("isPararContinuarEnabled").toString().toInt();
            XWR_Ajts->qdadeCrSalvos = xmlRead.attributes().value("qdadeCrSalvos").toString().toUInt();
            XWR_Ajts->salvDadosCarre = xmlRead.attributes().value("salvDadosCarre").toString().toUInt();
            XWR_Ajts->nomeArqConfTxT = xmlRead.attributes().value("nomeArqConfTxT").toString();
            xmlRead.skipCurrentElement();
        }
        else if (nodeName == "PSim")
        {
            XWR_Ajts->Dados.variaveis.qtSaidas = xmlRead.attributes().value("qtSaidas").toString().toInt();
            const QStringList nomes = splitTokens(xmlRead.attributes().value("nome").toString());
            for(int i=0; i<nomes.size(); i++) XWR_Ajts->Dados.variaveis.nome.push_back(nomes.at(i));
            const QStringList maiores = splitTokens(xmlRead.attributes().value("Vmaior").toString());
            for(int i=0; i<maiores.size(); i++) XWR_Ajts->Dados.variaveis.Vmaior.push_back(maiores.at(i).toDouble());
            const QStringList menores = splitTokens(xmlRead.attributes().value("Vmenor").toString());
            for(int i=0; i<menores.size(); i++) XWR_Ajts->Dados.variaveis.Vmenor.push_back(menores.at(i).toDouble());
            xmlRead.skipCurrentElement();
        }
        else if (nodeName == "valores")
        {
            const QStringList valores = splitTokens(xmlRead.attributes().value("valores").toString());
            for(int i=0; i<valores.size(); i++) XWR_Ajts->Dados.variaveis.valores.push_back(valores.at(i).toDouble());
            XWR_Ajts->Dados.variaveis.valores.setNumLinhas(xmlRead.attributes().value("linha").toString().toInt());
            XWR_Ajts->Dados.variaveis.valores.setNumColunas(xmlRead.attributes().value("coluna").toString().toInt());
            XWR_Ajts->Dados.iElitismo = xmlRead.attributes().value("iElitismo").toString().toInt();
            XWR_Ajts->Dados.qtdadeVarAnte = xmlRead.attributes().value("qtdadeVarAnte").toString().toInt();
            XWR_Ajts->Dados.timeInicial = xmlRead.attributes().value("timeInicial").toString().toDouble();
            XWR_Ajts->Dados.timeFinal = xmlRead.attributes().value("timeFinal").toString().toDouble();
            XWR_Ajts->Dados.isElitismo = xmlRead.attributes().value("isElitismo").toString().toInt();
            xmlRead.skipCurrentElement();
        }
        else if (nodeName.startsWith("Saida"))
        {
            QVector<Cromossomo> saidaPop;
            while (xmlRead.readNextStartElement())
            {
                if (!xmlRead.name().toString().startsWith("Cromossomo")) { xmlRead.skipCurrentElement(); continue; }

                Cromossomo cromossomo;
                cromossomo.aptidao = xmlRead.attributes().value("aptidao").toString().toDouble();
                cromossomo.erro = xmlRead.attributes().value("erro").toString().toDouble();
                cromossomo.idSaida = xmlRead.attributes().value("idSaida").toString().toInt();
                cromossomo.maiorAtraso = xmlRead.attributes().value("maiorAtraso").toString().toInt();

                while (xmlRead.readNextStartElement())
                {
                    if (!xmlRead.name().toString().startsWith("Regress")) { xmlRead.skipCurrentElement(); continue; }

                    cromossomo.err.append('C',xmlRead.attributes().value("Err").toString().toDouble());
                    cromossomo.vlrsCoefic.append('C',xmlRead.attributes().value("Coefic").toString().toDouble());
                    cromossomo.regress.append(QVector<compTermo>());

                    while (xmlRead.readNextStartElement())
                    {
                        if (!xmlRead.name().toString().startsWith("Termo")) { xmlRead.skipCurrentElement(); continue; }

                        compTermo termo;
                        termo.vTermo.tTermo1.var = xmlRead.attributes().value("var").toString().toUInt();
                        termo.vTermo.tTermo1.atraso = xmlRead.attributes().value("atraso").toString().toUInt();
                        termo.vTermo.tTermo1.reg = xmlRead.attributes().value("reg").toString().toUInt();
                        termo.vTermo.tTermo1.nd = xmlRead.attributes().value("nd").toString().toUInt();
                        termo.expoente = xmlRead.attributes().value("expoente").toString().toDouble();
                        cromossomo.regress.last().append(termo);
                        xmlRead.skipCurrentElement();
                    }
                }

                saidaPop.append(cromossomo);
            }
            XWR_Ajts->Pop.append(saidaPop);
        }
        else
        {
            xmlRead.skipCurrentElement();
        }
    }

    return !xmlRead.error();
}

QString XmlReaderWriter::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xmlRead.errorString())
            .arg(xmlRead.lineNumber())
            .arg(xmlRead.columnNumber());
}
