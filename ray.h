#ifndef RT_RAY_H
#define RT_RAY_H

#include "ysl_math/vect.h"
#include <array>

class ray
{
public:
	ray()
		: orig{}, dir{}, inv_dir{}, sign{0}, tm(0.0f)
	{}
	ray(const ray &r) : orig(r.orig), dir(r.dir), inv_dir(r.inv_dir), sign(r.sign), tm(r.tm) {}
    ray &operator=(const ray &r)
    {
        if (this != &r)
        {
            orig = r.orig;
            dir = r.dir;
            inv_dir = r.inv_dir;
            sign = r.sign;
            tm = r.tm;
        }

        return *this;
    }
	~ray() = default;

	ray(const ysl::point3 &origin, const ysl::vec3 &direction, f32 time = 0.0f)
		: orig(origin), dir(direction),
          tm(time)
	{
        calc_inv_dir(dir);
        sign[0] = (inv_dir[0] < 0);
        sign[1] = (inv_dir[1] < 0);
        sign[2] = (inv_dir[2] < 0);
    }

    void set_direction(const ysl::vec3 &new_dir)
    {
        dir = new_dir;
        calc_inv_dir(dir);
    }

    inline void calc_inv_dir(const ysl::vec3 &_dir)
    {
        //inv_dir = _dir.get_rcp();
        inv_dir.set_x(1 / dir[0]);
        inv_dir.set_y(1 / dir[1]);
        inv_dir.set_z(1 / dir[2]);
    }

    [[nodiscard]] ysl::point3 origin() const
    {
        return orig;
    }

    [[nodiscard]] ysl::vec3 direction() const
    {
        return dir;
    }

    [[nodiscard]] ysl::point3 at(f32 t) const
    {
        return orig + (t * dir);
    }

    [[nodiscard]] f32 time() const
    {
        return tm;
    }

    [[nodiscard]] ysl::vec3 inv_direction() const
    {
        return inv_dir;
    }

    [[nodiscard]] std::array<i32, 3> signs() const
    {
        return sign;
    }

    [[nodiscard]] ysl::vec3 vsign() const
    {
        return ysl::vec3{sign};
    }

private:
	ysl::point3 orig;
	ysl::vec3 dir;
    ysl::vec3 inv_dir;
    std::array<i32, 3> sign;
	f32 tm;
    
};

#endif
