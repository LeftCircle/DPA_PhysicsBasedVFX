/*
The majority of this code was generated automatically with Github Copilot using
ChatGPT mini
*/


#include <memory>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "field_interface.h"

namespace py = pybind11;
using namespace lux;

PYBIND11_MODULE(physics_vfx, module)
{
    module.doc() = "Physics-based VFX field operations";

    py::class_<Vector>(module, "Vector")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_property_readonly("x", &Vector::X)
        .def_property_readonly("y", &Vector::Y)
        .def_property_readonly("z", &Vector::Z)
        .def("magnitude", &Vector::magnitude)
        .def("unitvector", &Vector::unitvector)
        .def("__repr__", [](const Vector& value) {
            return "Vector(" +
                   std::to_string(value.X()) + ", " +
                   std::to_string(value.Y()) + ", " +
                   std::to_string(value.Z()) + ")";
        });

    py::class_<Volume<float>, std::shared_ptr<Volume<float>>> field(
        module,
        "Field"
    );

    field
        .def("eval", &Volume<float>::eval)
        .def("grad", &Volume<float>::grad)
        .def("set_fd_size", &Volume<float>::setFDSize)
        .def("set_fd_step",
             py::overload_cast<double>(&Volume<float>::setFDStep))
        .def("typelabel", &Volume<float>::typelabel);

    module.def(
        "sphere",
        &isf_sphere,
        py::arg("center"),
        py::arg("radius")
    );

    module.def(
        "torus",
        &isf_torus,
        py::arg("center"),
        py::arg("major_radius"),
        py::arg("minor_radius"),
        py::arg("normal")
    );

    module.def(
        "cone",
        &isf_cone,
        py::arg("origin"),
        py::arg("normal"),
        py::arg("height"),
        py::arg("theta")
    );

    module.def(
        "box",
        &isf_box,
        py::arg("center"),
        py::arg("radius"),
        py::arg("rounding_exponent")
    );

    module.def(
        "cylinder",
        &isf_cylinder,
        py::arg("center"),
        py::arg("normal"),
        py::arg("radius"),
        py::arg("height")
    );

    module.def("union_fields", &union_fields);
    module.def("intersection", &intersection);
    module.def("cutout", &cutout);
    module.def("mask", &mask);

    module.def(
        "translate",
        [](const vspf& field, const Vector& delta) {
            return lux::translate<float>(
                field,
                make_constant<Vector>(delta)
            );
        }
    );

    module.def(
        "negate",
        [](const vspf& field) {
            return -field;
        }
    );

    module.def(
        "function_field",
        [](py::function eval_function,
           py::object grad_function) -> vspf {
            FunctionField<float>::GradFunc gradient;

            if (!grad_function.is_none()) {
                gradient = [grad_function](const Vector& point) {
                    py::gil_scoped_acquire gil;
                    return grad_function(point).cast<Vector>();
                };
            }

            auto evaluator = [eval_function](const Vector& point) {
                py::gil_scoped_acquire gil;
                return eval_function(point).cast<float>();
            };

            return std::make_shared<FunctionField<float>>(
                std::move(evaluator),
                std::move(gradient)
            );
        },
        py::arg("eval_function"),
        py::arg("grad_function") = py::none()
    );
}