#include <iostream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <immintrin.h>
#include <stdint.h>

using namespace std;

template <class T>
class Matrix
{
private:
    double **MATRIX;
    size_t ROW, COL;

public:
    // The default constructor.
    Matrix()
    {
        MATRIX = nullptr;
        ROW = 0;
        COL = 0;
    }

    // The constructor without initial value.
    Matrix(size_t row, size_t col)
    {
        ROW = row;
        COL = col;
        if (!ROW || !COL)
        {
            // Case when row or column is 0, invalid matrix.
            return;
        }
        MATRIX = (double **)malloc(COL * sizeof(double *));
        double **begin = MATRIX;
        double **end = MATRIX + COL;
        while (begin != end)
        {
            // Allocate the memory for the matrix. No init val.
            *(begin++) = (double *)malloc(ROW * sizeof(double));
        }
    }

    // The constructor with initial value.
    Matrix(size_t row, size_t col, const T init_val)
    {
        ROW = row;
        COL = col;
        if (!ROW || !COL)
        {
            return;
        }
        MATRIX = (double **)malloc(COL * sizeof(int *));
        double **begin = MATRIX;
        double **end = MATRIX + COL;
        double *p1, *p2;

        while (begin != end)
        {
            p1 = *(begin++) = (double *)malloc(ROW * sizeof(double));
            p2 = p1 + ROW;
            while (p1 != p2)
            {
                // Assign the init_val to each entry of the matrix.
                *(p1++) = init_val;
            }
        }
    }

    double &operator()(size_t i, size_t j)
    {
        return MATRIX[j][i];
    }

    double operator()(size_t i, size_t j) const
    {
        return MATRIX[j][i];
    }

    void multiply_kernel(double **c, double **a, double **b, int row, int col)
    {
        __m256d t04_0, t04_1, t04_2, t04_3, t58_0, t58_1, t58_2, t58_3,
            a0, a1, b0, b1, b2, b3;
        t04_0 = t04_1 = t04_2 = t04_3 = t58_0 = t58_1 = t58_2 = t58_3 = _mm256_set1_pd(0);
        double *pb0(b[col]), *pb1(b[col + 1]), *pb2(b[col + 2]), *pb3(b[col + 3]), *pa0(a[0]), *pa1(a[1]), *endb0 = pb0 + COL;
        while (pb0 != endb0)
        {
            a0 = _mm256_loadu_pd(pa0);
            a1 = _mm256_loadu_pd(pa1);
            b0 = _mm256_set1_pd(*(pb0++));
            b1 = _mm256_set1_pd(*(pb1++));
            b2 = _mm256_set1_pd(*(pb2++));
            b3 = _mm256_set1_pd(*(pb3++));
            t04_0 += a0 * b0;
            t04_1 += a0 * b1;
            t04_2 += a0 * b2;
            t04_3 += a0 * b3;
            t58_0 += a1 * b0;
            t58_1 += a1 * b1;
            t58_2 += a1 * b2;
            t58_3 += a1 * b3;
            pa0 += 4;
            pa1 += 4;
        }
        _mm256_storeu_pd(&c[col][row], t04_0);
        _mm256_storeu_pd(&c[col + 1][row], t04_1);
        _mm256_storeu_pd(&c[col + 2][row], t04_2);
        _mm256_storeu_pd(&c[col + 3][row], t04_3);
        _mm256_storeu_pd(&c[col][row + 4], t58_0);
        _mm256_storeu_pd(&c[col + 1][row + 4], t58_1);
        _mm256_storeu_pd(&c[col + 2][row + 4], t58_2);
        _mm256_storeu_pd(&c[col + 3][row + 4], t58_3);
    }
    
    Matrix multiply(const Matrix &B)
    {
        if (COL != B.ROW)
            return *this;
        Matrix temp(ROW, B.COL, 0);
        double *ta[2];
        ta[0] = (double *)malloc(sizeof(double) * 4 * COL);
        ta[1] = (double *)malloc(sizeof(double) * 4 * COL);
        int i(0), j(0), k, t;
        do
        {
            k = 0;
            i = 0;
            do
            {
                ta[0][k] = MATRIX[i][j];
                ta[1][k++] = MATRIX[i][j + 4];
                ta[0][k] = MATRIX[i][j + 1];
                ta[1][k++] = MATRIX[i][j + 5];
                ta[0][k] = MATRIX[i][j + 2];
                ta[1][k++] = MATRIX[i][j + 6];
                ta[0][k] = MATRIX[i][j + 3];
                ta[1][k++] = MATRIX[i++][j + 7];
            } while (i < COL);
            i = 0;
            do
            {
                multiply_kernel(temp.MATRIX, ta, B.MATRIX, j, i);
                i += 4;
            } while (i < B.COL);
            j += 8;
        } while (j < ROW);
        free(ta[0]);
        free(ta[1]);
        return temp;
    }
};

int main()
{
    cout << "Size: " << sizeof(float) << endl;
    array<int, 10> test = {1000, 2000, 3000, 4000, 5000,
                           6000, 7000, 8000, 9000, 10000};
    cout << "Optimized, AVX2 required" << endl;
    cout << "Test Size: " << endl;
    for (auto it = test.begin(); it != test.end(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;

    for (int i = 0; i < test.size(); i++)
    {
        cout << test[i] << ": ";
        Matrix<float> A(test[i], test[i], 1.0);
        Matrix<float> B(test[i], test[i], 1.0);
        clock_t start = clock();
        A.multiply(B);
        clock_t end = clock();
        double time_duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        cout << time_duration << "s" << endl;
    }
    std::cout << "Done" << std::endl;
}
