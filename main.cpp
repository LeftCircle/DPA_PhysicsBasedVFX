#include <iostream>
#include <memory>

#include "command_line_parser.h"
#include "level_set_render_cuda.h"

using namespace lux;

const float PI = 3.14159265358979;

int main(int argc, char** argv){
	
	CommandLineParser parser(argc, argv);
	const std::string file_name = parser.find_val_for_flag("-image");
	if (file_name.empty()){
		std::cerr << "No image file name provided. Use -image <file_name> to make and save the image" << std::endl;
		return 1;
	}

	//render_bunny(file_name, "/bunny_rotated");
	// render_bust(file_name, "/ajax_small");
	//render_bunny_in_ajax(file_name, "/ajax_bun.exr");
	//render_humanoid_with_bunnybust(file_name, "/human");
	//render_bunny_in_ajax(file_name, "/human");
	//cuda_render_bunny(file_name, "/cuda_bunny");
	std::vector<CudaPointLight> lights{
		{make_float3(-3, 3, 1), make_float3(1, 0, 0), 100},
		{make_float3( 0, 0, -3), make_float3(0, 1, 0), 100},
		{make_float3( 3, -3, 1), make_float3(0, 0, 1), 100},
	};

	cuda_render_bunny(file_name, "cuda_wizard_bunny", lights);

	return 0;
}


