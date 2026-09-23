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

	
	raymarch_humanoid(file_name);

	return 0;
}


