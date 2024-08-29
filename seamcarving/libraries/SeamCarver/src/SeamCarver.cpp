#include "SeamCarver.hpp"

#include <Image.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stack>

SeamCarver::SeamCarver(Image image) : m_image(std::move(image)) {}

const Image &SeamCarver::GetImage() const {
    return m_image;
}

size_t SeamCarver::GetImageWidth() const {
    return m_image.m_table.size();
}

size_t SeamCarver::GetImageHeight() const {
    return m_image.m_table[0].size();
}

double Delta(Image::Pixel left, Image::Pixel right) {
    int r        = (left.m_red - right.m_red);
    int g        = (left.m_green - right.m_green);
    int b        = (left.m_blue - right.m_blue);
    double delta = r * r + g * g + b * b;
    return delta;
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const {
    Image::Pixel left =
        columnId == 0 ? m_image.GetPixel(GetImageWidth() - 1, rowId) : m_image.GetPixel(columnId - 1, rowId);
    Image::Pixel right =
        columnId == GetImageWidth() - 1 ? m_image.GetPixel(0, rowId) : m_image.GetPixel(columnId + 1, rowId);
    Image::Pixel up =
        rowId == 0 ? m_image.GetPixel(columnId, GetImageHeight() - 1) : m_image.GetPixel(columnId, rowId - 1);
    Image::Pixel down =
        rowId == GetImageHeight() - 1 ? m_image.GetPixel(columnId, 0) : m_image.GetPixel(columnId, rowId + 1);
    return std::sqrt(Delta(left, right) + Delta(up, down));
}

std::vector<size_t> BuildSeam(std::stack<size_t> st) {
    std::vector<size_t> seam;
    while (!st.empty()) {
        seam.push_back(st.top());
        st.pop();
    }
    return seam;
}

SeamCarver::Seam SeamCarver::FindSeam(bool isHorizontal) const {
    size_t height = GetImageHeight();
    size_t width  = GetImageWidth();
    std::vector<std::vector<double> > dp(width);
    size_t side1 = isHorizontal ? height : width;
    size_t side2 = isHorizontal ? width : height;
    std::fill(dp.begin(), dp.end(), std::vector<double>(height));
    for (size_t i = 0; i < side1; ++i) {
        isHorizontal ? dp[0][i] : dp[i][0] = isHorizontal ? GetPixelEnergy(0, i) : GetPixelEnergy(i, 0);
    }
    for (size_t i = 1; i < side2; ++i) {
        dp[i][0] = std::min(dp[i - 1][0], dp[i - 1][1]) + GetPixelEnergy(i, 0);
        for (size_t j = 1; j < side1 - 1; ++j) {
            dp[i][j] = std::min({dp[i - 1][j], dp[i - 1][j + 1], dp[i - 1][j - 1]}) + GetPixelEnergy(i, j);
        }
        dp[i][height - 1] = std::min(dp[i - 1][height - 1], dp[i - 1][height - 2]) + GetPixelEnergy(i, height - 1);
    }
    size_t start;
    double value = (double)INT64_MAX;
    for (size_t i = 0; i < side1; ++i) {
        value = std::min(value, dp[width - 1][i]);
        start = value == dp[width - 1][i] ? i : start;
    }
    std::stack<size_t> st;
    for (size_t i = side2 - 1; i > 0; --i) {
        st.push(start);
        double current = dp[i][start] - GetPixelEnergy(i, start);
        if (start != 0 && current == dp[i - 1][start - 1]) {
            --start;
        } else if (start != side1 - 1 && current == dp[i - 1][start + 1] && current != dp[i - 1][start]) {
            ++start;
        }
    }
    st.push(start);
    SeamCarver::Seam seam = BuildSeam(st);
    return seam;
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const {

}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const {
    size_t height = GetImageHeight();
    size_t width  = GetImageWidth();
    std::vector<std::vector<double> > dp(width);
    std::fill(dp.begin(), dp.end(), std::vector<double>(height));
    for (size_t i = 0; i < width; ++i) {
        dp[i][0] = GetPixelEnergy(i, 0);
    }
    for (size_t i = 1; i < height; ++i) {
        dp[0][i] = std::min(dp[0][i - 1], dp[1][i - 1]) + GetPixelEnergy(0, i);
        for (size_t j = 1; j < width - 1; ++j) {
            dp[j][i] = std::min({dp[j][i - 1], dp[j + 1][i - 1], dp[j - 1][i - 1]}) + GetPixelEnergy(j, i);
        }
        dp[width - 1][i] = std::min(dp[width - 1][i - 1], dp[width - 2][i - 1]) + GetPixelEnergy(width - 1, i);
    }
    size_t start;
    double value = (double)INT64_MAX;
    for (size_t i = 0; i < width; ++i) {
        value = std::min(value, dp[i][height - 1]);
        start = value == dp[i][height - 1] ? i : start;
    }
    std::stack<size_t> st;
    for (size_t i = height - 1; i > 0; --i) {
        st.push(start);
        double current = dp[start][i] - GetPixelEnergy(start, i);
        if (start != 0 && current == dp[start - 1][i - 1]) {
            --start;
        } else if (start != width - 1 && current == dp[start + 1][i - 1] && current != dp[start][i - 1]) {
            ++start;
        }
    }
    st.push(start);
    SeamCarver::Seam seam = BuildSeam(st);
    return seam;
}

void SeamCarver::RemoveSeam(const Seam &seam, bool isHorizontal) {
    for (size_t i = 0; i < (isHorizontal ? GetImageWidth() : GetImageHeight()); ++i) {
        for (size_t j = seam[i]; j < (isHorizontal ? GetImageHeight() : GetImageWidth()) - 1; ++j) {
            if (isHorizontal) {
                m_image.m_table[i][j] = m_image.m_table[i][j + 1];
            } else {
                m_image.m_table[j][i] = m_image.m_table[j + 1][i];
            }
        }
        if (isHorizontal) {
            m_image.m_table[i].pop_back();
        }
    }
    if (!isHorizontal) {
        m_image.m_table.pop_back();
    }
}

void SeamCarver::RemoveHorizontalSeam(const Seam &seam) {
    RemoveSeam(seam, true);
}

void SeamCarver::RemoveVerticalSeam(const Seam &seam) {
    RemoveSeam(seam, false);
}