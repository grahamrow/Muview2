// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Modified by Graham Rowlands <grahamrow@gmail.com>

#ifndef OMF_IMPORT_H
#define OMF_IMPORT_H

#include <QFile>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>
#include "matrix.h"

// Always using QSharedPointers to data arrays
// since they will be automatically deleted when
// going out of scope. This way the cache doesn't
// spiral out of control w.r.t memory usage

enum OMFFormat
{
    OMF_FORMAT_ASCII,
    OMF_FORMAT_BINARY_4,
    OMF_FORMAT_BINARY_8
};

class OMFReader : public QObject
{
    Q_OBJECT
public:
    OMFReader(QFile &fileref);
    bool read();
    QSharedPointer<matrix> field;

    // Header related
    QString Title;
    QStringList Desc;
    QStringList valueunits;  // OVF 2.0
    QStringList valuelabels; // OVF 2.0
    QString meshunit;        // e.g. "m"
    QString valueunit;       // e.g. "A/m"
    double valuemultiplier;
    double xmin, ymin, zmin;
    double xmax, ymax, zmax;
    double ValueRangeMaxMag, ValueRangeMinMag;
    QString meshtype;        // "rectangular"
    double xbase, ybase, zbase;
    double xstepsize, ystepsize, zstepsize;
    int xnodes, ynodes, znodes;
    int valuedim;            // OVF 2.0 only
    int version;

private:
    // Parsing related
    bool parse();
    bool parseFirstLine(QString &key, QString &value, int &version);
    bool parseSegment();
    bool parseHeader();
    bool parseCommentLine(QString &key, QString &value);
    bool parseDataAscii();
    bool parseDataBinary4();
    bool parseDataBinary8();
    void acceptLine();

    // OMFHeader header;
    QTextStream in;
    QString line;
    QString filename;
    QFile &file;
    int lineno;
};

QSharedPointer<OMFReader> readOMF(QString &path);

#endif

