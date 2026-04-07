#ifndef CSR_MATRIX_HPP
#define CSR_MATRIX_HPP

#include <vector>
#include <exception>

namespace sjtu {

class size_mismatch : public std::exception {
public:
    const char *what() const noexcept override {
        return "Size mismatch";
    }
};

class invalid_index : public std::exception {
public:
    const char *what() const noexcept override {
        return "Index out of range";
    }
};

template <typename T>
class CSRMatrix {

private:
    size_t n, m, count;
    std::vector<size_t> indptr;
    std::vector<size_t> indices;
    std::vector<T> data;
    
public:
    CSRMatrix &operator=(const CSRMatrix &other) = delete;
    CSRMatrix &operator=(CSRMatrix &&other) = delete;

    CSRMatrix(size_t n, size_t m) : n(n), m(m), count(0), indptr(n + 1, 0), indices(), data() {}

    CSRMatrix(size_t n, size_t m, size_t count,
        const std::vector<size_t> &indptr, 
        const std::vector<size_t> &indices,
        const std::vector<T> &data) : n(n), m(m), count(count), indptr(indptr), indices(indices), data(data) {
        if (indptr.size() != n + 1 || indices.size() != count || data.size() != count || indptr.back() != count) {
            throw size_mismatch();
        }
    }

    CSRMatrix(const CSRMatrix &other) = default;

    CSRMatrix(CSRMatrix &&other) = default;

    CSRMatrix(size_t n, size_t m, const std::vector<std::vector<T>> &data_in) : n(n), m(m), count(0) {
        if (data_in.size() != n) throw size_mismatch();
        indptr.resize(n + 1, 0);
        for (size_t i = 0; i < n; ++i) {
            if (data_in[i].size() != m) throw size_mismatch();
            for (size_t j = 0; j < m; ++j) {
                if (!(data_in[i][j] == T{})) {
                    indices.push_back(j);
                    data.push_back(data_in[i][j]);
                    count++;
                }
            }
            indptr[i + 1] = count;
        }
    }

    ~CSRMatrix() = default;

    size_t getRowSize() const { return n; }

    size_t getColSize() const { return m; }

    size_t getNonZeroCount() const { return count; }

    T get(size_t i, size_t j) const {
        if (i >= n || j >= m) throw invalid_index();
        for (size_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            if (indices[k] == j) return data[k];
        }
        return T{};
    }

    void set(size_t i, size_t j, const T &value) {
        if (i >= n || j >= m) throw invalid_index();
        for (size_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            if (indices[k] == j) {
                data[k] = value;
                return;
            }
            if (indices[k] > j) {
                if (!(value == T{})) {
                    indices.insert(indices.begin() + k, j);
                    data.insert(data.begin() + k, value);
                    for (size_t r = i + 1; r <= n; ++r) indptr[r]++;
                    count++;
                }
                return;
            }
        }
        if (!(value == T{})) {
            indices.insert(indices.begin() + indptr[i + 1], j);
            data.insert(data.begin() + indptr[i + 1], value);
            for (size_t r = i + 1; r <= n; ++r) indptr[r]++;
            count++;
        }
    }

    const std::vector<size_t> &getIndptr() const { return indptr; }

    const std::vector<size_t> &getIndices() const { return indices; }

    const std::vector<T> &getData() const { return data; }

    std::vector<std::vector<T>> getMatrix() const {
        std::vector<std::vector<T>> res(n, std::vector<T>(m, T{}));
        for (size_t i = 0; i < n; ++i) {
            for (size_t k = indptr[i]; k < indptr[i + 1]; ++k) {
                res[i][indices[k]] = data[k];
            }
        }
        return res;
    }

    std::vector<T> operator*(const std::vector<T> &vec) const {
        if (vec.size() != m) throw size_mismatch();
        std::vector<T> res(n, T{});
        for (size_t i = 0; i < n; ++i) {
            for (size_t k = indptr[i]; k < indptr[i + 1]; ++k) {
                res[i] = res[i] + data[k] * vec[indices[k]];
            }
        }
        return res;
    }

    CSRMatrix getRowSlice(size_t l, size_t r) const {
        if (l > r || r > n) throw invalid_index();
        size_t new_n = r - l;
        size_t new_count = indptr[r] - indptr[l];
        std::vector<size_t> new_indptr(new_n + 1, 0);
        std::vector<size_t> new_indices(new_count);
        std::vector<T> new_data(new_count);
        for (size_t i = 0; i < new_n; ++i) {
            new_indptr[i + 1] = indptr[l + i + 1] - indptr[l];
        }
        for (size_t k = 0; k < new_count; ++k) {
            new_indices[k] = indices[indptr[l] + k];
            new_data[k] = data[indptr[l] + k];
        }
        return CSRMatrix(new_n, m, new_count, new_indptr, new_indices, new_data);
    }
};

}

#endif // CSR_MATRIX_HPP