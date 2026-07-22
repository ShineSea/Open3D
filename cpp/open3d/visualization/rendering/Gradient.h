// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.open3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

#include <Eigen/Core>
#include <map>
#include <vector>
#include "open3d/geometry/Image.h"
#include "open3d/visualization/rendering/RendererHandle.h"

namespace open3d {
namespace visualization {
namespace rendering {

class Renderer;


class Gradient {
public:
    struct Point {
        float value;
        Eigen::Vector4f color;
    };

    Gradient();
    Gradient(const std::vector<Gradient::Point>& points);
    virtual ~Gradient();

    const std::vector<Gradient::Point>& GetPoints() const;
    void SetPoints(const std::vector<Gradient::Point>& points);

    enum class Mode {
        kGradient,  /// Normal gradient mode
        kLUT        /// Point.value will be ignored and the colors will be
                    /// assumed to be evenly spaced. The texture will have only
                    /// as many pixels as there are points.
    };

    Mode GetMode() const;
    void SetMode(Mode mode);

    /// Generates the gradient as a 512x1 RGBA8 Image.
    /// Pure CPU interpolation, no GPU dependency.
    std::shared_ptr<geometry::Image> GenerateImage() const;


    /// Legacy alias for CreateTexture().
    TextureHandle GetTextureHandle(Renderer& renderer);

private:
    std::vector<Gradient::Point> points_;
    Mode mode_ = Mode::kGradient;
    std::map<Renderer*, TextureHandle> textures_;
};

}  // namespace rendering
}  // namespace visualization
}  // namespace open3d
