/* bb_plane.h */

#ifndef BB_PLANE_H
#define BB_PLANE_H

#include "bb_param.h"
#include "core/object/object.h"

class BBPlane : public BBParam {
	GDCLASS(BBPlane, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PLANE; }
};

#endif // BB_PLANE_H