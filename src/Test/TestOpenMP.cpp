// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2015 Qianyi Zhou <Qianyi.Zhou@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include <cstdio>
#include <cmath>
#ifdef _OPENMP
#include <omp.h>
#endif 
#include <Core/Core.h>
#include <External/Eigen/Eigen/SVD>

#define NUM_THREADS 4
#define NUM_START 1
#define NUM_END 10

int main()
{
	int i = 0, nRet = 0, nSum = 0, nStart = NUM_START, nEnd = NUM_END;
	int nThreads = 1, nTmp = nStart + nEnd;
	unsigned uTmp = (unsigned(nEnd - nStart + 1) *
			unsigned(nTmp)) / 2;
	int nSumCalc = uTmp;

	if (nTmp < 0) {
		nSumCalc = -nSumCalc;
	}

#ifdef _OPENMP
	printf("OpenMP is supported.\n");
#else
	printf("OpenMP is not supported.\n");
#endif

#ifdef _OPENMP
	omp_set_num_threads(NUM_THREADS);
#endif

#pragma omp parallel default(none) private(i) shared(nSum, nThreads, nStart, nEnd)
	{
#ifdef _OPENMP
#pragma omp master
		nThreads = omp_get_num_threads();
#endif

#pragma omp for
		for (i = nStart; i <= nEnd; ++i) {
#pragma omp atomic
			nSum += i;
		}
	}

	if (nThreads == NUM_THREADS) {
		printf("%d OpenMP threads were used.\n", NUM_THREADS);
		nRet = 0;
	} else {
		printf("Expected %d OpenMP threads, but %d were used.\n",
				NUM_THREADS, nThreads);
		nRet = 1;
	}

	if (nSum != nSumCalc) {
		printf("The sum of %d through %d should be %d, "
				"but %d was reported!\n",
				NUM_START, NUM_END, nSumCalc, nSum);
		nRet = 1;
	} else {
		printf("The sum of %d through %d is %d\n",
				NUM_START, NUM_END, nSum);
	}

	for (int i = 1; i < 512; i *= 2) {
		char buff[1024];
		sprintf(buff, "simple task, %d tasks, %d threads", i, i);
		three::ScopeTimer t(buff);
#ifdef _OPENMP
		omp_set_num_threads(i);
#endif
#pragma omp parallel default(none) shared(nThreads)
		{
#ifdef _OPENMP
#pragma omp master
			nThreads = omp_get_num_threads();
#endif
			int n_a_rows = 2000;
			int n_a_cols = 2000;
			int n_b_rows = 2000;
			int n_b_cols = 2000;

			Eigen::MatrixXd a(n_a_rows, n_a_cols);
			for (int i = 0; i < n_a_rows; ++i)
				for (int j = 0; j < n_a_cols; ++j)
					a(i, j) = n_a_cols * i + j;

			Eigen::MatrixXd b(n_b_rows, n_b_cols);
			for (int i = 0; i < n_b_rows; ++i)
				for (int j = 0; j < n_b_cols; ++j)
					b(i, j) = n_b_cols * i + j;

			Eigen::MatrixXd d(n_a_rows, n_b_cols);
			d = a * b;
#ifdef _OPENMP
#pragma omp master
			three::PrintInfo("Output %f\n", d(0, 0));
#endif
		}
		three::PrintInfo("%d threads are used.\n", nThreads);
	}


	for (int i = 1; i < 512; i *= 2) {
		char buff[1024];
		sprintf(buff, "svd, %d tasks, %d threads", i, i);
		three::ScopeTimer t(buff);
#ifdef _OPENMP
		omp_set_num_threads(i);
#endif
#pragma omp parallel default(none) shared(nThreads)
		{
#ifdef _OPENMP
#pragma omp master
			nThreads = omp_get_num_threads();
#endif
			int n_a_rows = 10000;
			int n_a_cols = 200;
			Eigen::MatrixXd a(n_a_rows, n_a_cols);
			for (int i = 0; i < n_a_rows; ++i)
				for (int j = 0; j < n_a_cols; ++j)
					a(i, j) = n_a_cols * i + j;
			Eigen::JacobiSVD<Eigen::MatrixXd> svd(a, 
					Eigen::ComputeThinU | Eigen::ComputeThinV);
			Eigen::MatrixXd pca = svd.matrixU().block<10000, 10>(0, 0
					).transpose() * a;

#ifdef _OPENMP
#pragma omp master
			three::PrintInfo("Output %f\n", pca(0, 0));
#endif
		}
		three::PrintInfo("%d threads are used.\n", nThreads);
	}
}
