#include "humanoid.h"

namespace lux{

#define RADTODEG(radians) ((radians) * 180.0f / 3.14159265358979)
#define DEGTORAD(degrees) ((degrees * 3.14159265358979 / 180.0))

using vecsptr = std::shared_ptr<const std::vector<vspf>>;

//std::make_shared<const std::vector<vspf>>(std::initializer_list<vspf>{box, torus})

vspf isf_staff(){
    auto staff = isf_cylinder(Vector(), Vector(1, 0, 0), 0.1, 3);
    auto trans_staff = rotate_fixed(staff, Vector(0, 0, 1), DEGTORAD(85));
    // auto torus = isf_torus(Vector(-1.5, 0, 0), 1.0, 0.25, Vector(0, 0, 1));
    // staff = blinn_blend(std::make_shared<const std::vector<vspf>>(std::initializer_list<vspf>{-staff, -torus}), 0.1, 1);
    // staff = -staff;
    return trans_staff;
}

void raymarch_humanoid(const std::string& filename){
    ImageData test_image(1920 / 4, 1080 / 4, 4);
	RayMarcher rm;
	float near = 2.0;
    float far = 6.0;
    float ds = (far - near) / 60;
    rm.set_ds(ds);
	rm.set_snear(2);
	rm.set_sfar(6);
	rm.set_exticntion_coefficient(0.001);
	//rm.set_exticntion_coefficient(0.0);
	rm.set_Tmin(0.0001);
	
	Camera cam;
    // down Z
	cam.setEyeViewUp(Vector(0, 0, 5), Vector(0, 0, -1), Vector(0, 1, 0));
	// down X
    // cam.setEyeViewUp(Vector(-5, 0, 0), Vector(1, 0, 0), Vector(0, 1, 0));


    // // Head ---------------------------------
    // Vector top_head_pos = Vector(0, 1.0, 0);
    // float thr = 0.3;
    // auto top_head = isf_sphere(top_head_pos, thr);
    // auto chin = isf_ellipse(top_head_pos - Vector(0, thr * 0.65, 0), Vector(1, 0, 0), thr * 0.8, thr * 0.55);
    // auto head = union_fields(top_head, chin);

    // // Body ---------------------------------
    // Vector neck_point = top_head_pos - Vector(thr / 3.0, thr, 0);
    // float torso_length = thr * 2.25;
    // float torso_width = torso_length / 2.25;
    // auto torso = isf_ellipse(Vector(0, 0, 0), Vector(0, 1, 0), torso_length, torso_width);
    // torso = rotate_fixed(torso, Vector(0, 0, 1), DEGTORAD(22));
    // torso = rotate_fixed(torso, Vector(1, 0, 0), DEGTORAD(-15));
    // torso = translate_fixed(torso, Vector(-0.3, 0.2, -0.2));


    // // Legs ---------------------------------
    // float torso_base_adj_z = -0.25;
    // float thigh_length = torso_length * 0.75;
    // float thigh_width = 0.15;
    // auto left_thigh = isf_ellipse(Vector(), Vector(1, 0, 0), thigh_length, thigh_width);
    // left_thigh = rotate_fixed(left_thigh, Vector(0, 0, 1), DEGTORAD(45));
    // left_thigh = translate_fixed(left_thigh, Vector(-0.2, -0.6, torso_base_adj_z));

    // auto right_thigh = isf_ellipse(Vector(), Vector(1, 0, 0), thigh_length, thigh_width);
    // right_thigh = rotate_fixed(right_thigh, Vector(0, 0, 1), DEGTORAD(-35));
    // right_thigh = rotate_fixed(right_thigh, Vector(-1, 1, 0), DEGTORAD(-15));
    // right_thigh = translate_fixed(right_thigh, Vector(-0.83, -0.6, 0.1 + torso_base_adj_z));

    // auto left_shin = rotate_fixed(left_thigh, Vector(0, 0, 1), DEGTORAD(35));
    // left_shin = translate_fixed(left_shin, Vector(thigh_length, -thigh_length * 1.2, 0));

    // auto right_shin = isf_ellipse(Vector(), Vector(1, 0, 0), thigh_length, thigh_width * 0.8);
    // right_shin = rotate_fixed(right_shin, Vector(0, 0, 1), DEGTORAD(-55));
    // right_shin = translate_fixed(right_shin, Vector(-2.35 * thigh_length, -2 * thigh_length, 0));


    // // Arms ---------------------------------
    // float arm_length = torso_length * 0.5;
    // float arm_width = thigh_width * 0.65;
    // auto upper_right_arm = isf_ellipse(Vector(), Vector(1, 0, 0), arm_length, arm_width);
    // upper_right_arm = rotate_fixed(upper_right_arm, Vector(0, 0, 1), DEGTORAD(35));
    // upper_right_arm = translate_fixed(upper_right_arm, Vector(-1.8 * arm_length, torso_length, 0));

    // auto right_forearm = isf_ellipse(Vector(), Vector(1, 0, 0), arm_length, arm_width);
    // right_forearm = rotate_fixed(right_forearm, Vector(0, 0, 1), DEGTORAD(90));
    // right_forearm = translate_fixed(right_forearm, Vector(-2.3 * arm_length, torso_length + arm_length, 0));

    // auto left_arms = union_fields(right_forearm, upper_right_arm);
    // left_arms = translate_fixed(left_arms, Vector(2.7 * arm_length, -2.8 * arm_length, 0));
    // left_arms = rotate_fixed(left_arms, Vector(0, 0, 1), DEGTORAD(-15));

    // auto right_arm = union_fields(right_forearm, upper_right_arm);
    // auto arms = union_fields(right_arm, left_arms);
    // torso = union_fields(torso, arms);
    // auto thighs = union_fields(left_thigh, right_thigh);
    // auto shins = union_fields(left_shin, right_shin);
    // auto legs = union_fields(thighs, shins);
    // //auto legs = thighs;
    // //auto humanoid = union_fields(head, torso);
    // auto humanoid = union_fields(torso, legs);
    // humanoid = union_fields(humanoid, head);
    
    vspf staff = isf_staff();
    auto humanoid = staff;
    humanoid = -mask(humanoid);
	VolumeSPtr<Color> col = make_constant(Color(1, 0, 0, 0));

	//VolumeSPtr<Color> final_col = col * mask(-a) + col * mask(a);
	
	rm.ray_march_image(cam, test_image, humanoid, col);
	test_image.oiio_write_to(filename);

}

} // end namespace lux