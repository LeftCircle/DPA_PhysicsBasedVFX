#pragma once


#include "volume.h"


namespace lux{


class PlaneField : public Volume<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	PlaneField(const Vector& point, const Vector& normal) : _point(point), _normal(normal) {}
	PlaneField(Vector&& point, Vector&& normal) :
		_point(std::move(point)), _normal(std::move(normal)) {}

	const volumeDataType eval( const Vector& P ) const override { return _normal * (P - _point); }
    const volumeGradType grad( const Vector& P ) const override { return _normal; }
    virtual std::string typelabel() { return "Plane"; }	

private:
	Vector _point;
	Vector _normal;
};




} // end namespace lux





