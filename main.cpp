#include <iostream>
#include <memory>

#include "command_line_parser.h"
#include "humanoid.h"


using namespace lux;

const float PI = 3.14159265358979;

int main(int argc, char** argv){
	
	CommandLineParser parser(argc, argv);
	const std::string file_name = parser.find_val_for_flag("-image");
	if (file_name.empty()){
		std::cerr << "No image file name provided. Use -image <file_name> to make and save the image" << std::endl;
		return 1;
	}

	// ImageData test_image(1920 / 2, 1080 / 2, 4);
	// RayMarcher rm;
	// rm.set_ds(0.01);
	// rm.set_snear(0);
	// rm.set_sfar(6);
	// rm.set_exticntion_coefficient(0.01);
	// //rm.set_exticntion_coefficient(0.0);
	// rm.set_Tmin(0.0001);
	

	// Camera cam;
	// cam.setEyeViewUp(Vector(0, 0, 4), Vector(0, 0, -1), Vector(0, 1, 0));

	// // VolumeSPtr<float> a = isf_sphere(Vector(0, 0, 0), 0.5);
	// // auto a = isf_box(Vector(0, 0, 0), 0.5, 2);
	// // auto a = isf_cone(Vector(0, -0.5, 0), Vector(0, 1, 0), 1, 60 * PI / 360.0);
	// // auto a = isf_ellipse(Vector(), Vector(0, 1, 0), 0.5, 0.25);
	// // auto a = isf_icosahedron(Vector());
	// // a = scale(a, make_constant(0.1));
	// // auto a = isf_steiner_patch(Vector());
	// // auto a = isf_torus(Vector(), 0.5, 0.25, Vector(0, 1, 0));
	// // auto a = isf_cylinder(Vector(), Vector(0, 1, 0), 0.25, 0.5);

	// // a = union_fields(a, make_plane(Vector(), Vector(0, 1, 0)));
	// // a = cutout(a, make_plane(Vector(), Vector(0, 1, 0)));
	// // auto s1 = isf_sphere(Vector(-0.25, 0, 0), 0.5);
	// // auto s2 = isf_sphere(Vector(0.25, 0, 0), 0.5); 
	// // auto a = intersection(s1, s2);

	// // Blend works!
	// // auto torus = isf_torus(Vector(), 3.0, 0.4, Vector(0, 0, 1));
	// // auto box = isf_box(Vector(3, 0, 0), 1.0, 4);
	// // torus = -torus;
	// // box = -box;
	// // auto a = blinn_blend(std::make_shared<const std::vector<vspf>>(std::initializer_list<vspf>{box, torus}), 5.5, 1.0);
	// // a = -a;
	// //auto a = union_fields(torus, box);
	
	// // auto s1 = isf_sphere(Vector(-0.5, 0, 0), 0.5);
	// // auto s2 = isf_sphere(Vector(0.5, 0, 0), 0.5);
	// // s1 = -s1;
	// // s2 = -s2;
	// // //auto a = union_fields(s1, s2);
	// // auto a = s1 + s2 + blinn_blend(std::make_shared<const std::vector<vspf>>(std::initializer_list<vspf>{s1, s2}), -10, 0.0);

	// // auto a = isf_cone(Vector(0, -0.5, 0), Vector(0, 1, 0), 1, 60 * PI / 360.0);
	// // a = rotate(a, make_constant<Vector>(Vector(1, 1, 1)), make_constant<float>(90 * PI / 360.0));

	// // vspf a = isf_cone(Vector(0, -0.5, 0), Vector(0, 1, 0), 1, 60 * PI / 360.0);
	// // auto lin_y = funcfield<float>([](const Vector& p){
	// // 		return p.Y();
	// // 	},
	// // 	[](const Vector& p){
	// // 		return Vector(0, 1, 0);
	// // 	}
	// // );
	// // lin_y = scale(lin_y, make_constant(0.1));
	// // auto sinfield = sin(lin_y); 
	// // a = translate(a, make_constant(Vector(1, 0, 0)) * sinfield);

	// //vspf a = isf_cone(Vector(0, -0.5, 0), Vector(0, 1, 0), 1, 60 * PI / 360.0);
	// //a = scale(a, make_constant(0.25));

	// // auto c = isf_sphere(Vector(-2, 0, 0), 1);
	// // auto b = isf_sphere(Vector(2, 0, 0), 1);
	// // b = dilation(b, 0.5);

	// // auto a = union_fields(b, c);


	// // auto s = isf_sphere(Vector(), 3);
	// // s = shell(s, 0.01);
	// // auto p = make_plane(Vector(), Vector(-1, 0, -1));
	// // auto a = cutout(s, p);
	
	
	// auto a = isf_sphere(Vector(), 1);
	// a = -mask(a);
	// VolumeSPtr<Color> col = make_constant(Color(1, 0, 0, 0));

	// //VolumeSPtr<Color> final_col = col * mask(-a) + col * mask(a);
	
	// rm.ray_march_image(cam, test_image, a, col);
	// test_image.oiio_write_to(file_name);
	raymarch_humanoid(file_name);

	return 0;
}


