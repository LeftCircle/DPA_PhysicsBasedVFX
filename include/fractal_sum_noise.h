#pragma once


struct FSVals{
    float r;
    float octaves;
    Vector translation;
    float freq; 
    float fjump;
};

template <typename T, typename Vec, typename Func>
constexpr T fractal_sum(const Vec& eval_point, Func func, const FSVals&& vals){
    return fractal_sum(
        eval_point,
        func,
        vals.r,
        vals.octaves,
        vals.translation,
        vals.freq,
        vals.fjump
    );
}

template <typename T, typename Vec, typename Func>
constexpr T fractal_sum(
    const Vec& eval_point,
    Func func,
    float r, // roughness
    float octaves,
    const Vec& translation,
    float freq,
    float fjump
) {
    octaves = std::max(octaves, 1.0f);
    int n0 = (int)std::floor(octaves);
    auto nf = octaves - n0;
    auto scale = (1.0 - r) / (1.0 - std::pow(r, octaves));
    scale = r == 1 ? 1.0 / octaves : scale; // handle division by zero for r == 1;
    const auto x = eval_point - translation;
    T result{};
    for (int i = 0; i < n0; i++){
        result += std::pow(r, i) * func(x * freq * std::pow(fjump, i));
    }
    // add the fractional part
    result += nf * std::pow(r, octaves - 1) * func(x * freq * std::pow(fjump, octaves - 1));
    return scale * result;
}






