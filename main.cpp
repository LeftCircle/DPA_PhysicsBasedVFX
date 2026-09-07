#include <iostream>

#include "field_interface.h"
#include "ray_marcher.h"
#include "image_data.h"
#include "command_line_parser.h"


using namespace lux;

int main(int argc, char** argv){
	
	CommandLineParser parser(argc, argv);
	const std::string file_name = parser.find_val_for_flag("-image");
	if (file_name.empty()){
		std::cerr << "No image file name provided. Use -image <file_name> to make and save the image" << std::endl;
		return 1;
	}

	ImageData test_image(1920 / 2, 1080 / 2, 4);
	RayMarcher rm;
	rm.set_ds(0.003);
	rm.set_snear(0);
	rm.set_sfar(5);
	rm.set_exticntion_coefficient(0.0001);
	rm.set_Tmin(0.001);
	

	Camera cam;
	cam.setEyeViewUp(Vector(0, 0, 3), Vector(0, 0, -1), Vector(0, 1, 0));

	VolumeSPtr<float> a = isf_sphere(Vector(0, 0, 0), 0.5);
	a = -mask(a);
	VolumeSPtr<Color> col = make_constant(Color(1, 0, 0, 0));

	//VolumeSPtr<Color> final_col = col * mask(-a) + col * mask(a);
	
	rm.ray_march_image(cam, test_image, a, col);
	test_image.oiio_write_to(file_name);

	return 0;
}


