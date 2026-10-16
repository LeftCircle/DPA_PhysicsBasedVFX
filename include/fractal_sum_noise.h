#pragma once


template <typename T, typename Vec, typename Func>
constexpr T fractal_sum(
    Vec eval_point,
    Func func,
    float r, // roughness
    float octaves,
    Vec translation,
    float freq,
    float fjump
) {
    octaves = std::max(octaves, 1.0f);
    int n0 = (int)std::floor(octaves);
    auto nf = octaves - n0;
    auto scale = (1.0 - r) / (1 - std::pow(r, octaves));
    T result{};
    for (int i = 0; i < n0; i++){
        auto x = eval_point - translation;
        auto p1 = std::pow(r, i) * func(x * freq * std::pow(fjump, i));
        auto p2 = nf * std::pow(r, octaves - 1) * func(x * freq * std::pow(fjump, octaves - 1));
        result += p1 + p2;
    }
    return scale * result;
}







