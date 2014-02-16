#include "matrix.h"

matrix::matrix(int sizeX, int sizeY, int sizeZ)
{
    sizes << sizeX << sizeY << sizeZ;
    numElements = sizeX*sizeY*sizeZ;
    strides << 1 << sizeX << sizeY*sizeX;
    data = new QVector<QVector3D>(numElements);
}

void matrix::clear()
{
    data->fill(QVector3D(0,0,0));
}

void matrix::set(int x, int y, int z, QVector3D vector)
{
    data->replace(index(x,y,z), vector);
}

void matrix::set(int ind, QVector3D vector)
{
    data->replace(ind, vector);
}

QVector3D matrix::at(int x, int y, int z)
{
    return data->at(index(x,y,z));
}

QVector<int> matrix::shape()
{
    return sizes;
}

void matrix::minmaxScalar(float &min, float &max)
{
    float minSearch = data->at(0)[0];
    float maxSearch = data->at(0)[0];
    float val;

    for(int k=0; k<numElements; k++)
    {
        val = data->at(k)[0];
        if (data->at(k)[0] < minSearch) {
            minSearch = val;
        }
        if (data->at(k)[0] > maxSearch) {
            maxSearch = val;
        }
    }

    max = maxSearch;
    min = minSearch;
}

void matrix::minmaxMagnitude(float &min, float &max)
{
    float minSearch = data->at(0).length();
    float maxSearch = data->at(0).length();
    float length;

    for(int k=0; k<numElements; k++)
    {
        length = data->at(k).length();
        if (length < minSearch) {
            minSearch = length;
        }
        if (length > maxSearch) {
            maxSearch = length;
        }
    }

    max = maxSearch;
    min = minSearch;
}

int matrix::num_elements()
{
    return numElements;
}

int matrix::index(int x, int y, int z)
{
    return x*strides[0] + y*strides[1] + z*strides[2];
}
