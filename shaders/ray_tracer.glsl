#version 430 core   

layout (local_size_x = 8, local_size_y = 8) in;             
layout (rgba32f, binding = 0) restrict writeonly uniform image2D img_output;   

out vec4 frag_color;
uniform float time;
uniform int spp;
uniform bool pause_light;
uniform bool show_more_mats;

#define MAX_DEPTH 3
//#define SPP 8
#define TMAX 9999999.

#define pi 3.14159265358979
#define pi_180 0.0174532925
#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439  // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503

const int mat_lambertian = 0;
const int mat_dielectric = 1;
const int mat_metal = 2;
const int mat_light_diffuse = 3;

// plane types
const int plane_yz = 0;
const int plane_zx = 1;
const int plane_xy = 2;


struct Material
{
    vec4 albedo;
    float fuzz_ir;
    int type;
};

struct Sphere
{
    Material mat;
    vec3 center;
    float radius;
};

struct Box
{
    mat4 world;
    Material mat;
    vec3 pmin;
    vec3 pmax;
};

struct Plane
{
    Material mat;
    vec4 corners;
    float k;
    int type;
    bool flip_face;
};

struct Ray 
{
    vec3 origin;
    vec3 direction;
};

struct Camera
{
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 v_fwd;
	vec3 v_side;
	vec3 v_up;
	float lens_radius;
};

struct HitResult
{
    vec3 p;
    vec3 normal;
    vec2 uv;
    Material mat;
    float t;
    bool front_face;
};

struct ScatterResult
{
    Ray out_ray;
    vec3 attenuation;
    bool is_specular;
};

layout(std430, binding = 0) buffer MatObjects
{
    Material mat_array[];
};

layout(std430, binding = 1) buffer PlaneObjects
{
    Plane plane_array[];
};

layout(std430, binding = 2) buffer CameraObject
{
	Camera cam;
};

uniform int mat_count;
uniform int plane_count;

// GLOBAL FOR TESTING
const Material light = Material(vec4(25., 25., 25.,0.), 0.0, mat_light_diffuse);
Sphere sph = Sphere(light, vec3(250., 370., 100.), 40.0); // light
const Plane pl = Plane(light, vec4(213.-100., 343.-100., 227.-100., 332.-100.), 552., plane_zx, true);

float hash12(vec2 p) {
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);

}
vec3 hash32(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}

vec3 random_in_unit_sphere(vec2 p) {
    vec3 rand = hash32(p);
    float phi = 2.0 * pi * rand.x;
    float cosTheta = 2.0 * rand.y - 1.0;
    float u = rand.z;
    
    float theta = acos(cosTheta);
    float r = pow(u, 1.0 / 3.0);
    
    float x = r * sin(theta) * cos(phi);
    float y = r * sin(theta) * sin(phi);
    float z = r * cos(theta);

    return vec3(x, y, z);

}

vec3 random_cos_direction(const vec3 n, vec2 seed)
{
    vec2 r = hash22(seed*999.);
    
	vec3  uu = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  vv = cross( uu, n );
	
	float ra = sqrt(r.y);
	float rx = ra*cos(6.2831*r.x); 
	float ry = ra*sin(6.2831*r.x);
	float rz = sqrt( 1.0-r.y );
	vec3  rr = vec3( rx*uu + ry*vv + rz*n );
    
    return normalize( rr );
}

vec3 random_cos_2(vec2 seed)
{
    vec2 r = hash22(seed * 999.);
    float z = sqrt(1.-r.y);
    float phi = 2. * pi * r.x;
    float x = cos(phi) * sqrt(r.y);
    float y= sin(phi) * sqrt(r.y);
    return vec3(x,y,z);
}

vec3 random_unit_vector(vec2 p) {
    return normalize(random_in_unit_sphere(p));
}

vec3 random_in_unit_disk(vec2 p) {
    return vec3(random_in_unit_sphere(p).xy, 0);
}

vec3 randomSphereDirection(vec2 seed) {
    vec2 h = hash22(seed*999.) * vec2(2.,6.28318530718)-vec2(1.,0.);
    float phi = h.y;
	return vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi),cos(phi)),h.x);
}

vec3 random_to_sphere(float radius, float dist_sqrd, vec2 seed)
{
    vec2 r = hash22(seed * 999.);
    float z= 1. + r.y * (sqrt(1.- radius*radius/dist_sqrd) - 1.);

    float phi = 2. * pi * r.x;
    float x = cos(phi) * sqrt(1.-z*z);
    float y= sin(phi) * sqrt(1.-z*z);

    return vec3(x,y,z);
}

vec3 construct_onb(vec3 vec, vec3 dir)
{
    vec3 axis[3];
    axis[2] = normalize(vec);
    vec3 a = (abs(axis[2].x) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
    axis[1] = normalize(cross(axis[2], a));
    axis[0] = cross(axis[2], axis[1]);

    return (dir.x * axis[0] + dir.y * axis[1] + dir.z * axis[2]);
}

// Camera
Camera make_camera(vec3 look_from, vec3 look_at, vec3 v_up_world, 
                   float v_fov, float aspect_ratio, float aperture,
                   float focus_dist)
{
    Camera cam;
    float theta = radians(v_fov);
    float h = tan(theta / 2.0);
    float viewport_height = 2.0 * h;
    float viewport_width = aspect_ratio * viewport_height;

    cam.v_fwd = normalize(look_from - look_at);
    cam.v_side = normalize(cross(v_up_world, cam.v_fwd));
    cam.v_up = cross(cam.v_fwd, cam.v_side);

    cam.origin = look_from;
    cam.horizontal = focus_dist * viewport_width * cam.v_side;
    cam.vertical = focus_dist * viewport_height * cam.v_up;
    cam.lower_left_corner = cam.origin - cam.horizontal / 2.0 - cam.vertical / 2.0 - focus_dist * cam.v_fwd;

    cam.lens_radius = aperture / 2.0;

    return cam;
}

Ray get_ray(Camera cam, vec2 seed)
{
    vec3 rd = cam.lens_radius * random_in_unit_disk(seed * 999. + 0. );
    vec3 offset = cam.v_side * rd.x + cam.v_up * rd.y;

    return Ray(cam.origin + offset,
               (cam.lower_left_corner + seed.x * cam.horizontal + seed.y * cam.vertical - cam.origin - offset));
}

bool set_front_face(Ray r_in, vec3 out_norm, out vec3 normal)
{
    bool front_face = dot(r_in.direction, out_norm) < 0.0;
    normal = front_face ? out_norm : -out_norm;
    return front_face;
}

bool near_zero(vec3 p)
{
    float s = 1e-8;
    return p.x < s && p.y < s && p.z < s;
}

float reflectance(float cosine, float ref_idx)
{
    // Schlick's approximation
    float r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

vec3 cone_uniform(float theta, vec3 dir, vec2 seed)
{
    vec3 left = cross(dir, vec3(0,1,0));
    left = length(left) > 0.1 ? normalize(left) : normalize(cross(dir, vec3(0,0,1)));
    vec3 up = normalize(cross(dir, left));

    vec2 u = hash22(seed+ time);
    float cos_theta = (1. - u.x) + u.x * cos(theta);
    float sin_theta = sqrt(1.-cos_theta * cos_theta);
    float phi = u.y * 2. * pi;
    return normalize(left * cos(phi) * sin_theta + up * sin(phi) * sin_theta + dir * cos_theta);

}


float scatter_pdf(Ray r_in, HitResult hit_in, Ray scattered)
{
    Material m = hit_in.mat;
    if(m.type == mat_lambertian)
    {
        float cosine = dot(normalize(scattered.direction), hit_in.normal);
        return cosine < 0.0 ? 0.0 : cosine/pi;
    }
    
    return 0.;
}

// Material
bool scatter(HitResult hit_in, Ray r_in, vec2 seed, out ScatterResult s_out)
{
    Material m = hit_in.mat;
    if(m.type == mat_lambertian)
    {
        s_out.attenuation = m.albedo.xyz;
        s_out.is_specular = false;
        s_out.out_ray = Ray(hit_in.p, construct_onb(hit_in.normal, random_cos_2(seed)));
    }
    else if (m.type == mat_metal)
    {
        vec3 reflected = reflect(normalize(r_in.direction), hit_in.normal);
        s_out.out_ray = Ray(hit_in.p, reflected + m.fuzz_ir * random_in_unit_sphere(seed));
        s_out.attenuation = m.albedo.xyz;
        s_out.is_specular = true;
    }
    else if(m.type == mat_dielectric)
    {
        s_out.is_specular = true;
        s_out.attenuation = vec3(1);

        float refract_ratio = hit_in.front_face ? (1.0 / m.fuzz_ir) : m.fuzz_ir;
        vec3 unit_dir = normalize(r_in.direction);
        float cos_theta = min(dot(-unit_dir, hit_in.normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refract_ratio * sin_theta > 1.0;
        vec3 direction;
        if(cannot_refract || reflectance(cos_theta, refract_ratio) > hash12(seed))
        {
            direction = reflect(unit_dir, hit_in.normal);
        }
        else
        {
            direction = refract(unit_dir, hit_in.normal, refract_ratio);
        }

        s_out.out_ray = Ray(hit_in.p, direction);

    }
    else
    {
        return false; // if light
    }

    return true;
}

vec3 emitted(Ray r_in, vec2 uv, vec3 p, HitResult hit_in)
{
    Material m = hit_in.mat;

    if(m.type == mat_light_diffuse)
    {
        if(hit_in.front_face)
        {
            return m.albedo.xyz;
        }
    }

    return vec3(0,0,0);
}  

// hit functions
bool intersect_ray_sphere(Sphere obj, float t_min, float t_max, Ray r_in, out HitResult hit_out)
{
    vec3 oc = r_in.origin - obj.center;
    float a = dot(r_in.direction, r_in.direction);
    float half_b = dot(oc, r_in.direction);
    float c = dot(oc, oc) - (obj.radius * obj.radius);

    float disc = half_b * half_b - a * c;
    if(disc < 0.) return false;

    float sqrtd = sqrt(disc);

    float root = (-half_b - sqrtd) / a;
    if(root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if(root < t_min || t_max < root)
        {
            return false;
        }
    }

    hit_out.t = root;
    hit_out.p = r_in.origin + (root * r_in.direction);
    vec3 out_normal = (hit_out.p - obj.center) / obj.radius;
    hit_out.front_face = set_front_face(r_in, out_normal, hit_out.normal);
    hit_out.mat = obj.mat;

    return true;
}

bool intersect_ray_aabb(Ray r_in, float t_min, float t_max, vec3 pmin, vec3 pmax, inout HitResult hit_out)
{
    vec3 v_min = (pmin - r_in.origin)/r_in.direction;
    vec3 v_max = (pmax - r_in.origin)/r_in.direction;
    vec3 sc = min(v_min, v_max);
    vec3 sf = max(v_min, v_max);
    float t0 = max(max(sc.x, sc.y), sc.z);
    float t1 = min(min(sf.x, sf.y), sf.z);
    if(!(t0 <= t1 && t1 > t_min)) return false;
    if(t0 > t_max) return false;
    
    // normals
    hit_out.normal = -sign(r_in.direction)*step(sc.yzx, sc.xyz)*step(sc.zxy, sc.xyz);
    hit_out.t = t0;
    hit_out.p = r_in.origin + (t0 * r_in.direction);
    hit_out.front_face = set_front_face(r_in, hit_out.normal, hit_out.normal);

    return true;
}

bool intersect_ray_obb(Box obj, Ray r_in, float t_min, float t_max, inout HitResult hit_out)
{
    mat4 invm = inverse(obj.world);

    vec3 r_orig = (invm * vec4(r_in.origin, 1)).xyz;
    vec3 r_dir = (invm * vec4(r_in.direction, 0)).xyz;

    Ray r = Ray(r_orig, r_dir);

    bool result = intersect_ray_aabb(r, t_min, t_max, obj.pmin, obj.pmax, hit_out);
    if(!result) return false;

    mat4 nm = transpose(invm);
    hit_out.p = (obj.world * vec4(hit_out.p,1)).xyz;
    hit_out.normal = normalize((nm*vec4(hit_out.normal, 0)).xyz);
    hit_out.mat = obj.mat;

    return true;
}

bool intersect_ray_plane(Plane obj, Ray r_in, float t_min, float t_max, out HitResult hit_out)
{
    int s3_idx = obj.type;
    float t = (obj.k - r_in.origin[s3_idx]) / r_in.direction[s3_idx];
    if(t < t_min || t > t_max) 
    {
        return false;
    }

    vec3 p = r_in.origin + t * r_in.direction;
    hit_out.p = p;

    vec2 s1 = obj.corners.xy;
    vec2 s2 = obj.corners.zw;

    int s1_idx = (s3_idx+1) % 3;
    int s2_idx = (s3_idx+2) % 3;

    if(p[s1_idx] < s1[0] || p[s1_idx] > s1[1] || p[s2_idx] < s2[0] || p[s2_idx] > s2[1])
    {
        return false;
    }

    hit_out.uv.x = (p[s1_idx] - s1[0]) / (s1[1] - s1[0]);
    hit_out.uv.y = (p[s2_idx] - s2[0]) / (s2[1] - s2[0]);
    hit_out.t = t;
    vec3 norm;
    norm[s3_idx] = 1.;
    hit_out.front_face = set_front_face(r_in, norm, hit_out.normal);
    
    if(obj.flip_face) hit_out.front_face = !hit_out.front_face;

    hit_out.mat = obj.mat;

    return true;
}

// hit
bool hit(Ray r_in, float t_min, float t_max, out HitResult hit_out)
{
    Material red        = mat_array[0];
    Material green      = mat_array[1];
    Material white      = mat_array[2];
    Material aluminum   = mat_array[3];
    Material glass      = mat_array[4];
    Material blue       = mat_array[5];
    Material cyan     = mat_array[6];

    mat4 mm = mat4(0.95,0.,0.309,0.,
                   0.,1.,0.,0.,
                   -.309017,0.,0.95105,0.,
                   20.,0.,-20.,1.);

    mat4 m2 = mat4(0.945518, 0.0, -0.325568, 0.0,
                     0.0, 1.0, 0.0, 0.0,
                     0.32556814, 0.0, 0.945518553, 0.0,
                     -85.0, 0.0, 60.0, 1.0);
    
    Box box1 = Box(mm, white, vec3(130, 0, 65), vec3(295, 165, 230));
    Box box2;
    Sphere sph1;
    if(show_more_mats)
    {
        box2 = Box(m2, aluminum, vec3(265., 0.0, 295.), vec3(430., 330., 460.));
        sph1 = Sphere(glass, vec3(210., 200., 130.), 35.);
    }
    else
    {
        box2 = Box(m2, white, vec3(265., 0.0, 295.), vec3(430., 330., 460.));
        sph1 = Sphere(green, vec3(210., 200., 130.), 35.);
    }
    
    Box boxes[2] = Box[](box1, box2);

    vec3 ball_pos = vec3(370., 35.0, 100.);
    if(!pause_light)
    {
        ball_pos = vec3(370., 35.0, 100.) + vec3(100. * sin(time), 0, 20. * cos(time));
    }
    Sphere ball = Sphere(cyan, ball_pos, 30.);
    Sphere spheres[3] = Sphere[](sph, sph1, ball);
    

    //Plane planes[6] = Plane[](p1, p2, p3, p4, p5, pl);
    //Plane planes[5] = Plane[](p1, p2, p3, p4, p5);
    bool hit_anything = false;
    float closest_so_far = TMAX;
    HitResult temp_hit;

    // boxes
    for(int i = 0; i < 2; i++)
    {
        if(intersect_ray_obb(boxes[i], r_in, t_min, closest_so_far, temp_hit))
        {
            hit_anything = true;
            closest_so_far = temp_hit.t;
            hit_out = temp_hit;
        }
    }

    // planes
    for(int i = 0; i < 6; i++)
    {
        if(intersect_ray_plane(plane_array[i], r_in, t_min, closest_so_far, temp_hit))
        {
            hit_anything = true;
            closest_so_far = temp_hit.t;
            hit_out = temp_hit;
        }
    }

    //if(hit_anything) return true;

    // spheres
    for(int i = 0; i < 3; i++)
    {
        if(intersect_ray_sphere(spheres[i], t_min, closest_so_far, r_in, temp_hit))
        {
            hit_anything = true;
            closest_so_far = temp_hit.t;
            hit_out = temp_hit;
          }
    }
    // Boxes
    //bool result = false;
    //result = intersect_ray_obb(box1, r_in, TMAX, hit_out);
    //return result;
    return hit_anything;
} 

Ray plane_sample_light(vec2 seed, HitResult hit, int depth)
{
    vec2  rr = hash22(seed*9999.);
    float xy = pl.corners.x + (pl.corners.y - pl.corners.x) * rr.x;
    float zw = pl.corners.z + (pl.corners.w - pl.corners.z) * rr.y;
    vec3 new_dir = vec3(xy, pl.k, zw);
    return Ray(hit.p, new_dir - hit.p);
}

float plane_pdf(Ray r)
{
    HitResult hit_temp;
    if(!hit(r, 0.001, TMAX, hit_temp))
    {
        return 0.;
    }
            

    vec2 s1 = pl.corners.xy;
    vec2 s2 = pl.corners.zw;

    float area = (s1.y - s1.x) * (s2.y - s2.x);
    float dist_sqr = hit_temp.t * hit_temp.t * dot(r.direction,r.direction);
    float coss = abs(dot(r.direction, hit_temp.normal) / sqrt(dot(r.direction,r.direction)));

    return dist_sqr / (coss * area);
}

float sphere_pdf(Ray r, out vec3 normal)
{
    HitResult hit_temp;
    if(!intersect_ray_sphere(sph, 0.001, TMAX, r, hit_temp))//if(!hit(r, 0.001, TMAX, hit_temp))
    {
        return 0.;
    }
    normal = hit_temp.normal;
    float cos_theta_max = sqrt(1. - (sph.radius*sph.radius / (dot(sph.center - r.origin,sph.center - r.origin))));
    float solid_angle = 2. * pi  * (1. - cos_theta_max);

    return 1. / solid_angle;
}

Ray sphere_sample_light(vec2 seed, vec3 origin)
{
    vec3 dir = sph.center - origin;
    float dist_sqr = dot(dir, dir);
    return Ray(origin, construct_onb(dir, random_to_sphere(sph.radius, dist_sqr, seed)));
}

vec3 trace_ray(Ray r, vec3 background, vec2 seed)
{
    vec3 color = vec3(1.);
    
    HitResult hit_out;
    ScatterResult s_out;
    
    int depth = 0;
    int spec_bounce = 4;
    for(depth = 0; depth < MAX_DEPTH; depth++)
    {
        if(hit(r, 0.001, TMAX, hit_out))
        {
            vec3 emitted = emitted(r, vec2(0.), hit_out.p, hit_out);

            // if false we hit a light
            if(!scatter(hit_out, r, seed*999. + float(depth) , s_out))
            {
                color *= emitted;
                break;
            }

            if(s_out.is_specular)
            {
                r = s_out.out_ray;
                color *= s_out.attenuation;
                if(spec_bounce > 0) depth--; spec_bounce--;
                continue;
            }   
            
            //float pdf_value = cosine / pi;
            float pdf_value;
            if(hash12(seed*999. + float(depth)) < 0.5)
            {
               s_out.out_ray = sphere_sample_light(seed, s_out.out_ray.origin);
               //color = vec3(0.65, 0.2, 0.2);
               //break;
            }
            //s_out.out_ray = plane_sample_light(seed, hit_out, depth);
            
            float cosine = dot(normalize(s_out.out_ray.direction), hit_out.normal);
           
            vec3 normal;
            pdf_value = (0.5 * ((cosine <= 0.0) ? 0.0 : cosine/pi)) + (0.5 * sphere_pdf(s_out.out_ray, normal));
            //pdf_value = (cosine <= 0.0) ? 0.0 : cosine/pi;
            //pdf_value = plane_pdf(s_out.out_ray);
            //color *= (s_out.attenuation * scatter_pdf(r, hit_out, s_out.out_ray) * sph.mat.albedo) * (weight * clamp(dot(nld, hit_out.normal), 0., 1.));
            color *= s_out.attenuation * scatter_pdf(r, hit_out, s_out.out_ray) / pdf_value;
            
            r = s_out.out_ray;
        }
        else
        {
            color *= background;
            break;
        }
    }

    if(depth == MAX_DEPTH)
    {
        return vec3(0.,0.,0.);
    }

    return color; // return color/pdf_value;
}

void main()
{
    vec4 pixel = vec4 (0.0, 0.0, 0.0, 1.0);                 
    vec2 pixel_coords = gl_GlobalInvocationID.xy;  
                                   
    vec2 dims = vec2(imageSize (img_output));

    // seed for random ray direction
    vec2 rand;
    vec2 normalizedCoord;

    // move sphere
    if(!pause_light)
    {
        sph.center = vec3(250., 370., 100.) + vec3(100. * sin(time), 100. * sin(time), 100. * cos(time));
    }

    
    Ray r;

    for(int i = 0; i < spp; i++)
    {
        rand = hash22(pixel_coords.xy * 999. + float(i) + time);
        //sph.center -= vec3(4.0, 0.0, 0.0);
        
        normalizedCoord = (pixel_coords.xy + rand) / dims.xy;
        r = get_ray(cam, normalizedCoord);
        pixel += vec4(trace_ray(r, vec3(0.0), normalizedCoord), 1.);
       
    }

    // gamma correction
    pixel.xyz /= float(spp);
    pixel.xyz = pow(clamp(pixel.xyz, 0.0, 1.0), vec3(0.45));

    // Output to screen
    imageStore (img_output, ivec2(pixel_coords), pixel);     
    //frag_color = vec4(pixel.xyz, 1.);
}
