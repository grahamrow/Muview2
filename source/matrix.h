#ifndef MATRIX_H
#define MATRIX_H
#include <QVector>
#include <QVector3D>

class matrix
{
public:
    matrix(int sizeX, int sizeY, int sizeZ);
    void clear();
    void set(int x, int y, int z, QVector3D vector);
    void set(int ind, QVector3D vector);
    QVector3D at(int x, int y, int z);
    QVector<int> shape();
    void minmaxScalar(float &min, float &max);
    void minmaxMagnitude(float &min, float &max);
    int num_elements();
private:
    int index(int x, int y, int z);
    // x, y, z ordering
    QVector<int> sizes;
    QVector<int> strides;
    int numElements;
    QVector<QVector3D> *data;
};

#endif // MATRIX_H
