
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
	
	

	VolumeSPtr<float> a = isf_sphere(Vector(), 1.0);

	return 0;
}


