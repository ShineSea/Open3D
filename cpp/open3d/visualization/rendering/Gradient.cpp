// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.open3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "open3d/visualization/rendering/Gradient.h"

#include "open3d/utility/Logging.h"
#include "open3d/visualization/rendering/Renderer.h"

namespace open3d {
namespace visualization {
namespace rendering {

namespace {
static const int kNGradientPixels = 512;
static const int kNChannels = 4;

void SetPixel(std::vector<uint8_t>& img,
              int idx,
              const Eigen::Vector4f& color) {
    int i = kNChannels * idx;
    img[i] = uint8_t(std::round(255.0f * color[0]));
    img[i + 1] = uint8_t(std::round(255.0f * color[1]));
    img[i + 2] = uint8_t(std::round(255.0f * color[2]));
    img[i + 3] = uint8_t(std::round(255.0f * color[3]));
}

}  // namespace

Gradient::Gradient() {}

Gradient::Gradient(const std::vector<Gradient::Point>& points) {
    points_ = points;
}

Gradient::~Gradient() {}

const std::vector<Gradient::Point>& Gradient::GetPoints() const {
    return points_;
}

void Gradient::SetPoints(const std::vector<Gradient::Point>& points) {
    points_ = points;
    for (size_t i = 0; i < points_.size(); ++i) {
        if (points_[i].value < 0.0f || points_[i].value > 1.0f) {
            utility::LogWarning(
                    "Gradient point {} must be in range [0.0, 1.0], clamping",
                    points_[i].value);
            points_[i].value = std::max(0.0f, std::min(1.0f, points_[i].value));
        }
    }
    textures_.clear();
}

Gradient::Mode Gradient::GetMode() const { return mode_; }

void Gradient::SetMode(Mode mode) {
    if (mode != mode_) {
        mode_ = mode;
        textures_.clear();
    }
}

std::shared_ptr<geometry::Image> Gradient::GenerateImage() const {
    auto img = std::make_shared<geometry::Image>();

    if (points_.empty()) {
        img->Prepare(1, 1, kNChannels, 1);
        SetPixel(img->data_, 0, {1.0f, 0.0f, 1.0f, 1.0f});  // magenta fallback
        return img;
    }

    int n_points = int(points_.size());

    if (mode_ == Mode::kLUT) {
        // LUT mode: one pixel per point, no interpolation
        img->Prepare(n_points, 1, kNChannels, 1);
        for (int i = 0; i < n_points; ++i) {
            SetPixel(img->data_, i, points_[i].color);
        }
    } else {
        // Gradient mode: smooth interpolation over kNGradientPixels
        img->Prepare(kNGradientPixels, 1, kNChannels, 1);
        float n = float(kNGradientPixels - 1);
        int idx = 0;

        for (int img_idx = 0; img_idx < kNGradientPixels; ++img_idx) {
            float x = float(img_idx) / n;
            while (idx < n_points && x > points_[idx].value) {
                ++idx;
            }

            if (idx == 0) {
                SetPixel(img->data_, img_idx, points_[0].color);
            } else if (idx == n_points) {
                SetPixel(img->data_,
                         img_idx,
                         points_[n_points - 1].color);
            } else {
                const auto& p0 = points_[idx - 1];
                const auto& p1 = points_[idx];
                float dist = p1.value - p0.value;
                float w1 = (x - p0.value) / dist;
                float w0 = 1.0f - w1;
                Eigen::Vector4f color;
                color[0] = w0 * p0.color[0] + w1 * p1.color[0];
                color[1] = w0 * p0.color[1] + w1 * p1.color[1];
                color[2] = w0 * p0.color[2] + w1 * p1.color[2];
                color[3] = w0 * p0.color[3] + w1 * p1.color[3];
                SetPixel(img->data_, img_idx, color);
            }
        }
    }

    return img;
}

TextureHandle Gradient::GetTextureHandle(Renderer& renderer) {
    if (textures_.find(&renderer) == textures_.end()) {
        auto img = GenerateImage();
        textures_[&renderer] = renderer.AddTexture(img);
    }
    return textures_[&renderer];
}

}  // namespace rendering
}  // namespace visualization
}  // namespace open3d
