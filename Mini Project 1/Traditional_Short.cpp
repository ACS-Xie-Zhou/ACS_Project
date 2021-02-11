#include <iostream>
#include <cstdlib>
#include <ctime>
#include <array>

using namespace std;

template <class T>
class Matrix
{
private:
    T **_Matrix;
    size_t _Row, _Col;

public:
    // The default constructor.
    Matrix()
    {
        _Matrix = nullptr;
        _Row = 0;
        _Col = 0;
    }

    // The constructor without initial value.
    Matrix(size_t row, size_t col)
    {
        _Row = row;
        _Col = col;
        if (!_Row || !_Col)
        {
            // Case when row or column is 0, invalid matrix.
            return;
        }
        _Matrix = (T **)malloc(_Col * sizeof(T *));
        T **begin = _Matrix;
        T **end = _Matrix + _Col;
        while (begin != end)
        {
            // Allocate the memory for the matrix. No init val.
            *(begin++) = (T *)malloc(_Row * sizeof(T));
        }
    }

    // The constructor with initial value.
    Matrix(size_t row, size_t col, const T init_val)
    {
        _Row = row;
        _Col = col;
        if (!_Row || !_Col)
        {
            return;
        }
        _Matrix = (T **)malloc(_Col * sizeof(T *));
        T **begin = _Matrix;
        T **end = _Matrix + _Col;
        T *p1, *p2;

        while (begin != end)
        {
            p1 = *(begin++) = (T *)malloc(_Row * sizeof(T));
            p2 = p1 + _Row;
            while (p1 != p2)
            {
                // Assign the init_val to each entry of the matrix.
                *(p1++) = init_val;
            }
        }
    }

    T &operator()(size_t i, size_t j)
    {
        return _Matrix[j][i];
    }

    T operator()(size_t i, size_t j) const
    {
        return _Matrix[j][i];
    }

    Matrix multiply(const Matrix &B)
    {
        if (_Col != B._Row)
        {
            return *this;
        }
        Matrix temp(_Row, B._Col, 0);
        T i, j(0), k;
        while (j < B._Col)
        {
            i = 0;
            while (i < _Row)
            {
                k = 0;
                while (k < _Col)
                {
                    temp(i, j) += (*this)(i, k) * B(k, j);
                    k++;
                }
                i++;
            }
            j++;
        }
        return temp;
    }
};

int main()
{
    cout << "Size: " << sizeof(short) << endl;
    array<int, 10> test = {1000, 2000, 3000, 4000, 5000,
                           6000, 7000, 8000, 9000, 10000};
    cout << "Traditional, no SIMD" << endl;
    cout << "Test Size: " << endl;
    for (auto it = test.begin(); it != test.end(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;

    for (int i = 0; i < test.size(); i++)
    {
        cout << test[i] << ": ";
        Matrix<short> A(test[i], test[i], 1);
        Matrix<short> B(test[i], test[i], 1);
        clock_t start = clock();
        A.multiply(B);
        clock_t end = clock();
        double time_duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        cout << time_duration << "s" << endl;
    }
    std::cout << "Done" << std::endl;
}
