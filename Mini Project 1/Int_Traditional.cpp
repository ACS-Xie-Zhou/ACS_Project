#include <iostream>
#include <cstdlib>
#include <ctime>
#include <array>

using namespace std;

class Matrix {
private:
    int **_Matrix;
    size_t _Row, _Col;

public:
    Matrix() {
        _Matrix = nullptr;
        _Row = 0;
        _Col = 0;
    }

    Matrix(size_t row, size_t col) {
        _Row = row;
        _Col = col;
        if (!_Col || !_Row) {
            return;
        }
        _Matrix = (int **) malloc(_Col * sizeof(int *));
        int **p = _Matrix, **end = _Matrix + _Col;
        do {
            *(p++) = (int *) malloc(_Row * sizeof(int));
        } while (p != end);
    }

    Matrix(size_t row, size_t col, const int init_val) {
        _Row = row;
        _Col = col;
        if (!_Col || !_Row) {
            return;
        }
        _Matrix = (int **) malloc(_Col * sizeof(int *));
        int **begin = _Matrix, **end = _Matrix + _Col, *p1, *p2;
        do {
            p1 = *(begin++) = (int *) malloc(_Row * sizeof(int));
            p2 = p1 + _Row;
            do {
                *(p1++) = init_val;
            } while (p1 != p2);
        } while (begin != end);
    }

    int &operator()(size_t i, size_t j) {
        return _Matrix[j][i];
    }

    int operator()(size_t i, size_t j) const {
        return _Matrix[j][i];
    }

    void multi_kernel(int **c, int **a, int **b, int row, int col) const {
        int t0(0), t1(0), t2(0), t3(0), t4(0), t5(0), t6(0), t7(0),
                t8(0), t9(0), t10(0), t11(0), t12(0), t13(0), t14(0), t15(0);
        int *a0(a[0]), *a1(a[1]), *a2(a[2]), *a3(a[3]),
                *b0(b[col]), *b1(b[col + 1]), *b2(b[col + 2]), *b3(b[col + 3]), *end = b0 + _Row;
        do {
            t0 += *(a0) * *(b0);
            t1 += *(a0) * *(b1);
            t2 += *(a0) * *(b2);
            t3 += *(a0++) * *(b3);
            t4 += *(a1) * *(b0);
            t5 += *(a1) * *(b1);
            t6 += *(a1) * *(b2);
            t7 += *(a1++) * *(b3);
            t8 += *(a2) * *(b0);
            t9 += *(a2) * *(b1);
            t10 += *(a2) * *(b2);
            t11 += *(a2++) * *(b3);
            t12 += *(a3) * *(b0++);
            t13 += *(a3) * *(b1++);
            t14 += *(a3) * *(b2++);
            t15 += *(a3++) * *(b3++);
        } while (b0 != end);
        c[col][row] = t0;
        c[col + 1][row] = t1;
        c[col + 2][row] = t2;
        c[col + 3][row] = t3;
        c[col][row + 1] = t4;
        c[col + 1][row + 1] = t5;
        c[col + 2][row + 1] = t6;
        c[col + 3][row + 1] = t7;
        c[col][row + 2] = t8;
        c[col + 1][row + 2] = t9;
        c[col + 2][row + 2] = t10;
        c[col + 3][row + 2] = t11;
        c[col][row + 3] = t12;
        c[col + 1][row + 3] = t13;
        c[col + 2][row + 3] = t14;
        c[col + 3][row + 3] = t15;
    }

    Matrix multiply(const Matrix &B) {
        if (_Col != B._Row) {
            return *this;
        }
        Matrix temp(_Row, B._Col, 0);
        int *tr[4];
        int i(0), j(0);
        do {
            tr[i++] = (int *) malloc(sizeof(int) * _Col);
        } while (i < 4);
        do {
            i = 0;
            do {
                tr[0][i] = _Matrix[i][j];
                tr[1][i] = _Matrix[i][j + 1];
                tr[2][i] = _Matrix[i][j + 2];
                tr[3][i] = _Matrix[i][j + 3];
            } while ((++i) < _Col);
            i = 0;
            do {
                multi_kernel(temp._Matrix, tr, B._Matrix, j, i);
                i += 4;
            } while (i < B._Col);
            j += 4;
        } while (j < _Row);
        return temp;
    }
};

int main() {
    array<int, 10> test = {1000,
                           2000,
                           3000,
                           4000,
                           5000,
                           6000,
                           7000,
                           8000,
                           9000,
                           10000};
    cout << "integer, traditional " << test.size() << endl;
    for (int i = 0; i < test.size(); i++) {
        clock_t start = clock();
        cout << test[i] << ": ";
        Matrix A(test[i], test[i], 1);
        Matrix B(test[i], test[i], 1);
        A.multiply(B);
        clock_t end = clock();
        double time_duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        cout << time_duration << "s" << endl;
    }
    std::cout << "Done" << std::endl;
}
