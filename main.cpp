#include <iostream>

#include "field_interface.h"
#include "ray_marcher.h"
#include "image_data.h"



using namespace lux;

int main(int argc, char** argv){

	ImageData test_image(64, 64, 4);
	RayMarcher rm;
	rm.set_ds(0.01);
	rm.set_snear(0);
	rm.set_sfar(3);
	rm.set_exticntion_coefficient(1.0);
	rm.set_Tmin(0.01);
	

	Camera cam;

	VolumeSPtr<float> a = isf_sphere(Vector(0, 0, 0), 0.4);
	VolumeSPtr<Color> col = make_constant(Color(1, 0, 0, 0));

	//VolumeSPtr<Color> final_col = col * mask(-a) + col * mask(a);
	
	rm.ray_march_image(cam, test_image, a, col);
	test_image.oiio_write_to("images/test_image.exr");

	return 0;
}


