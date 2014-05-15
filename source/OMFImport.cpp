// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Converted to Qt libs and custom QVector based matrix class
// by Graham Rowlands <grahamrow@gmail.com>

#include <QSharedPointer>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDataStream>
#include <QDebug>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "matrix.h"
#include "OMFImport.h"
#include "OMFHeader.h"

QSharedPointer<matrix> readOMF(QString &path, OMFHeader &header)
{
    bool success;
    QFile file(path);
    OMFReader reader(file);
    success = reader.read();
    if (!success) {
        return QSharedPointer<matrix>();
    } else {
        header = reader.getHeader();
        return QSharedPointer<matrix>(reader.getField());
    }
    return QSharedPointer<matrix>();
}

OMFHeader OMFReader::getHeader()
{
    return header;
}

QSharedPointer<matrix> OMFReader::getField()
{
    return field;
}

OMFReader::OMFReader(QFile &fileref) : file(fileref)
{

}

bool OMFReader::read()
{
    bool ok;
    QString key, value;

    if (!file.open(QIODevice::ReadOnly))
    {
        return QSharedPointer<matrix>();
    }
    // Read first line
    acceptLine();
    ok = parseFirstLine(key, value, header.version);
    if (ok && key == "oommf") {
        acceptLine();
    } else {
        qDebug() << "Expected valid 'OOMMF' header at line 1";
        return false;
    }

    ok = parseCommentLine(key, value);
    if (ok && key == "segment count") {
        acceptLine();
    } else {
        qDebug() << "Expected 'Segment count' at line 2";
        return false;
    }

    ok = parseCommentLine(key, value);
    if (ok && key == "begin" && value == "segment") {
        parseSegment();
    } else {
        qDebug() << "Expected begin of segment";
        return false;
    }

    return true;
}

bool OMFReader::parseFirstLine(QString &key, QString &value, int &version)
{
    if (line=="# OOMMF: OVF 2.0") {
        key="oommf";
        value="ovf 2.0";
        version=2;
        return true;
    } else if (line=="# OOMMF: rectangular mesh v1.0") {
        key="oommf";
        value="rectangular mesh v1.0";
        version=1;
        return true;
    } else if (line=="# OOMMF OVF 2.0") {
        key="oommf";
        value="ovf 2.0";
        version=2;
        return true;
    } else {
        return false;
    }
    return false;
}

bool OMFReader::parseCommentLine(QString &key, QString &value)
{
    if (line.at(0) == '#') {
        int sep = line.indexOf(':');
        key     = line.mid(2, sep-2).toLower();
        value   = line.mid(sep+2).toLower();
        return true;
    } else {
        return false;
    }
}

void OMFReader::acceptLine()
{
    bool reallydone = false;
    while(!reallydone && !file.atEnd())
    {
        line = file.readLine();
        if( line.endsWith("\n") ) line.truncate( line.length() - 1 );
        if (line=="#") {
            reallydone=false;
        } else if (line=="") {
            reallydone=true;
        } else if (line.isNull()) {
            reallydone=true;
        } else {
            reallydone=true;
        }
    }
}

bool OMFReader::parseSegment()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "segment") {
        qDebug() << "Parse error. Expected 'Begin Segment'";
        return false;
    }
    acceptLine();

    parseHeader();
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin") {
        qDebug() << "Parse error. Expected 'Begin Data <type>'";
        return false;
    }
    if (value == "data text") {
        ok = parseDataAscii();
    } else if (value == "data binary 4") {
        ok = parseDataBinary4();
    } else if (value == "data binary 8") {
        ok = parseDataBinary8();
    } else {
        qDebug() << "Expected either 'Text', 'Binary 4' or 'Binary 8' chunk type";
        return false;
    }

    if (!ok) {
        qDebug() << "Parsing failed. May load anyway!";
        return false;
    }
#ifndef _WIN32
    ok = parseCommentLine(key, value);
    if (!ok || key != "end" || value != "segment") {
        qDebug() << "Expected 'End Segment'";
        return false;
    }
    acceptLine();
#endif
    return true;
}

bool OMFReader::parseHeader()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "header") {
        qDebug() << "Expected 'Begin Header'";
    }
    acceptLine();

    bool done = false;
    while (!done) {
        ok = parseCommentLine(key, value);
        if (!ok) {
            qDebug() << "Skipped erroneous line in header.";
            continue;
        }

        if (key == "end" && value == "header") {
            done = true;
            break;
        } else if (key == "title") {
            header.Title = value;
        } else if (key == "desc") {
            header.Desc.push_back(value);
        } else if (key == "meshunit") {
            header.meshunit = value;
        } else if (key == "valueunit") {
            header.valueunit = value;
        } else if (key == "valuemultiplier") {
            header.valuemultiplier = value.toDouble();
        } else if (key == "xmin") {
            header.xmin = value.toDouble();
        } else if (key == "ymin") {
            header.ymin = value.toDouble();
        } else if (key == "zmin") {
            header.zmin = value.toDouble();
        } else if (key == "xmax") {
            header.xmax = value.toDouble();
        } else if (key == "ymax") {
            header.ymax = value.toDouble();
        } else if (key == "zmax") {
            header.zmax = value.toDouble();
        } else if (key == "valuedim") {   // OVF 2.0
            header.valuedim = value.toInt();
        } else if (key == "valueunits") { // OVF 2.0
            header.valueunits.push_back(value);
        } else if (key == "valuelabels") { // OVF 2.0
            header.valuelabels.push_back(value);
        } else if (key == "valuerangeminmag") {
            header.ValueRangeMinMag = value.toDouble();
        } else if (key == "valuerangemaxmag") {
            header.ValueRangeMaxMag = value.toDouble();
        } else if (key == "meshtype") {
            header.meshtype = value;
        } else if (key == "xbase") {
            header.xbase = value.toDouble();
        } else if (key == "ybase") {
            header.ybase = value.toDouble();
        } else if (key == "zbase") {
            header.zbase = value.toDouble();
        } else if (key == "xstepsize") {
            header.xstepsize = value.toDouble();
        } else if (key == "ystepsize") {
            header.ystepsize = value.toDouble();
        } else if (key == "zstepsize") {
            header.zstepsize = value.toDouble();
        } else if (key == "xnodes") {
            header.xnodes = value.toInt();
        } else if (key == "ynodes") {
            header.ynodes = value.toInt();
        } else if (key == "znodes") {
            header.znodes = value.toInt();
        } else {
            qDebug() << "OMFReader::parseHeader: Unknown key: " << key << "---" << value;
        }
        acceptLine();
    }

    if (header.version == 1) {
        header.valuedim = 3;
    }

    ok = parseCommentLine( key, value);
    if (!ok || key != "end" || value != "header") {
        qDebug() << "Expected 'End Header'";
        return false;
    }
    acceptLine();
    return true;
}

bool OMFReader::parseDataAscii()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data text") {
        qDebug() << "Expected 'Begin DataText'";
        return false;
    }
    acceptLine();

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));

    for (int z=0; z<header.znodes; ++z)
        for (int y=0; y<header.ynodes; ++y)
            for (int x=0; x<header.xnodes; ++x) {
                QTextStream ss(&line);

                double v1, v2, v3;
                QVector3D val;
                if (header.valuedim == 1) {
                    ss >> v1;
                    val = QVector3D(v1,v1,v1);
                } else {
                    ss >> v1 >> v2 >> v3;
                    val = QVector3D(v1,v2,v3);
                }

                if (header.version==1) {
                    val = val*header.valuemultiplier;
                }

                field->set(x,y,z,val);

                acceptLine();
            }
    // The following code causes problems on Windows only.
    // For now, just comment out!
#ifndef _WIN32
    ok = parseCommentLine(key, value);
    if (!ok || key != "end" || value != "data text") {
        qDebug() << "Expected 'End Data Text'";
        return false;
    }
    acceptLine();
#endif
    return true;
}

bool OMFReader::parseDataBinary4()
{
    Q_ASSERT(sizeof(float) == 4);

    bool ok;
    QString key, value;

    // Parse "Begin: Data Binary 4"
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data binary 4") {
        qDebug() << "Expected 'Begin Binary 4'";
        return false;
    }

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));
    const int num_cells = field->num_elements();

    // Read magic value and field contents from file
    double magic;
    QByteArray magicArray = file.read(sizeof(float));
    QDataStream magicStream(magicArray);
    magicStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if (header.version==1) {
        magicStream.setByteOrder(QDataStream::BigEndian);
        magicStream >> magic;
    } else if (header.version==2) {
        magicStream.setByteOrder(QDataStream::LittleEndian);
        magicStream >> magic;
    } else {
        qDebug() << "Wrong version number detected.";
        return false;
    }

    if (magic != 1234567.0) qDebug() << "Wrong magic number (binary 4 format)";

    QByteArray dataArray;
    if (header.valuedim == 1) {
        dataArray = file.read(num_cells*sizeof(float));
    } else {
        dataArray = file.read(3*num_cells*sizeof(float));
    }
    QDataStream dataStream(dataArray);
    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if (header.version==1) {
        dataStream.setByteOrder(QDataStream::BigEndian);
    } else {
        dataStream.setByteOrder(QDataStream::LittleEndian);
    }

    QVector3D val;
    double v1, v2, v3;
    for (int i=0; i<num_cells; ++i) {
        if (header.valuedim == 1) {
            dataStream >> v1;
            if (header.version==1) {
                v1 = v1 * header.valuemultiplier;
            }
            val = QVector3D(v1,v1,v1);
        } else {
            dataStream >> v1 >> v2 >> v3;
            val = QVector3D(v1, v2, v3);
            if (header.version==1) {
                val = val * header.valuemultiplier;
            }
        }
        field->set(i,val);
    }
    return true;
}

bool OMFReader::parseDataBinary8()
{
    Q_ASSERT(sizeof(double) == 8);

    bool ok;
    QString key, value;

    // Parse "Begin: Data Binary 8"
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data binary 8") {
        qDebug() << "Expected 'Begin Binary 8'";
        return false;
    }

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));
    const int num_cells = field->num_elements();

    // Read magic value and field contents from file
    double magic;
    QByteArray magicArray = file.read(sizeof(double));
    QDataStream magicStream(magicArray);
    magicStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    if (header.version==1) {
        magicStream.setByteOrder(QDataStream::BigEndian);
        magicStream >> magic;
    } else {
        magicStream.setByteOrder(QDataStream::LittleEndian);
        magicStream >> magic;
    }

    if (magic != 123456789012345.0) qDebug() << "Wrong magic number (binary 8 format)";

    QByteArray dataArray;
    if (header.valuedim == 1) {
        dataArray = file.read(num_cells*sizeof(double));
    } else {
        dataArray = file.read(3*num_cells*sizeof(double));
    }
    QDataStream dataStream(dataArray);
    dataStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    if (header.version==1) {
        dataStream.setByteOrder(QDataStream::BigEndian);
    } else {
        dataStream.setByteOrder(QDataStream::LittleEndian);
    }

    QVector3D val;
    double v1, v2, v3;
    for (int i=0; i<num_cells; ++i) {
        if (header.valuedim == 1) {
            dataStream >> v1;
            if (header.version==1) {
                v1 = v1 * header.valuemultiplier;
            }
            val = QVector3D(v1,v1,v1);
        } else {
            dataStream >> v1 >> v2 >> v3;
            val = QVector3D(v1, v2, v3);
            if (header.version==1) {
                val = val * header.valuemultiplier;
            }
        }
        field->set(i,val);
    }
    return true;
}

