# Physics-Based VFX: Volume Rendering and Fields

A C++ and CUDA project for building, manipulating, and rendering volumetric
fields. It combines analytic implicit surfaces, procedural noise and wisps,
OpenVDB grids, and a ray-marching renderer with Python bindings and USD scene
loading (in progress).

| Key Areas | Relevant project work | Where to look |
| --- | --- | --- |
| Modern C++ and mathematical foundations | Templated field operations, vector and matrix transforms, implicit functions, stochastic sampling, and numerical integration connect C++ implementation with calculus, linear algebra, and probability. | [`include/volume.h`](include/volume.h), [`include/field_operations.h`](include/field_operations.h), [`include/implicit_fields.h`](include/implicit_fields.h) |
| Physically based volume rendering | CPU ray marching accumulates color and exponential transmittance through a density field, with point lights and deep shadow maps. A separate CUDA ray marcher implements a related GPU path. | [`src/ray_marcher.cpp`](src/ray_marcher.cpp), [`src/deep_shadow_map.cpp`](src/deep_shadow_map.cpp), [`src/cuda/ray_marcher_cuda.cu`](src/cuda/ray_marcher_cuda.cu) |
| Rendering efficiency and thin-volume sampling | Randomized ray and shadow stepping, sparse OpenVDB grids, and a CUDA implementation offer places to study sampling cost, resolution and quality. The current renderer uses ray marching; it does not implement delta tracking or path guiding. | [`src/ray_marcher.cpp`](src/ray_marcher.cpp), [`include/deep_shadow_map.h`](include/deep_shadow_map.h), [`src/openvdb_helpers.cpp`](src/openvdb_helpers.cpp) |
| Volume appearance and procedural looks | Noise fields, fractal sums, and wisp construction provide controls for shaping volumetric detail and exploring pyro and wisp looks. | [`include/noise_fields.h`](include/noise_fields.h), [`include/fractal_sum_noise.h`](include/fractal_sum_noise.h), [`include/wisps.h`](include/wisps.h) |
| Surface and volume representations | Analytic implicit surfaces and converted OBJ/USD meshes can be represented as scalar fields and used to construct volume densities. This is a useful basis for exploring surface/volume relationships, not a unified surface, subsurface, and volume transport model. | [`include/implicit_fields.h`](include/implicit_fields.h), [`include/field_interface.h`](include/field_interface.h), [`python/usd_mesh.py`](python/usd_mesh.py) |
| Verification and debugging | Catch2 tests and Python tests cover field behavior, scene loading, image output and module import. | [`tests/`](tests/), [`python/`](python/), [`CMakeLists.txt`](CMakeLists.txt) |





