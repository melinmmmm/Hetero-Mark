#ifndef PageRank_H
#define PageRank_H

#include <stdio.h>/* for printf */
#include <stdint.h>/* for uint64 definition */
#include <stdlib.h>/* for exit() definition */
#include <time.h>/* for clock_gettime */
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <time.h>

#include "kernels.h"

#define BILLION 1000000000L

class PageRank
{
private:
	void InitBuffer();
	void FillBuffer();
	void FillBufferCpu();
	void FillBufferGpu();
	void ExecKernel();

	void FreeBuffer();
	void ReadBuffer();

	void ReadCsrMatrix();
	void ReadDenseVector();
	void PageRankCpu();

	std::string fileName1;
	std::string fileName2;
	int nnz;
	int nr;

	int* rowOffset;
	int* col;
	float* val;
	float* vector;
	float* eigenV;

	std::ifstream csrMatrix;
	std::ifstream denseVector;
	size_t global_work_size[1];
	size_t local_work_size[1];
	int workGroupSize;
	int maxIter;
	int isVectorGiven;

public:
	PageRank();
	PageRank(std::string, std::string);
	PageRank(std::string);
	~PageRank();

	void Run();
	void CpuRun();
	void Test();
	float* GetEigenV();
	void Print();
	void PrintOutput();
	int GetLength();
	float abs(float);
	
};

#endif