#include <iostream>
#include "OMFContainer.h"

void minmax(array_ptr array, float &min, float &max) {
	float minSearch = (float)(*array)[0][0][0][0];;
	float maxSearch = (float)(*array)[0][0][0][0];;

	const long unsigned int *size = array->shape();
	int xnodes = size[0];
	int ynodes = size[1];
	int znodes = size[2];
	for(int i=0; i<xnodes; i++)
	{
		for(int j=0; j<ynodes; j++)
		{
			for(int k=0; k<znodes; k++)
			{
				for(int v=0; v<3; v++) {
					if ((*array)[i][j][k][v] < minSearch) { 
						minSearch = (float)(*array)[i][j][k][v];
					}
					if ((*array)[i][j][k][v] > maxSearch) { 
						maxSearch = (float)(*array)[i][j][k][v];
					}
				}
			}
		}
	}

	max = maxSearch;
	min = minSearch;
	// std::cout << "Max value\t" << max << std::endl;
	// std::cout << "Min value\t" << min << std::endl;
}

void minmaxmag(array_ptr array, float &min, float &max) {
	float mag = sqrt( (*array)[0][0][0][0] * (*array)[0][0][0][0] +
		            (*array)[0][0][0][1] * (*array)[0][0][0][1] +
		            (*array)[0][0][0][2] * (*array)[0][0][0][2]);
	float minSearch = mag;
	float maxSearch = mag;


	const long unsigned int *size = array->shape();
	int xnodes = size[0];
	int ynodes = size[1];
	int znodes = size[2];
	for(int i=0; i<xnodes; i++)
	{
		for(int j=0; j<ynodes; j++)
		{
			for(int k=0; k<znodes; k++)
			{
				mag = sqrt( (*array)[i][j][k][0] * (*array)[i][j][k][0] +
				            (*array)[i][j][k][1] * (*array)[i][j][k][1] +
				            (*array)[i][j][k][2] * (*array)[i][j][k][2]);

				if (mag < minSearch) { 
					minSearch = (float)mag;
				}
				if (mag > maxSearch) { 
					maxSearch = (float)mag;
				}

			}
		}
	}
	max = maxSearch;
	min = minSearch;
	// std::cout << "Max value\t" << max << std::endl;
	// std::cout << "Min value\t" << min << std::endl;
}