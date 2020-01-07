
#include <iostream>
// #include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ASSERT(X) if(!(X)) { __debugbreak();}


#pragma pack(push, 1)
struct bitmap_header{
	uint16_t  FileType;     /* File type, always 4D42h ("BM") */
	uint32_t  FileSize;     /* Size of the file in bytes */
	uint16_t  Reserved1;    /* Always 0 */
	uint16_t  Reserved2;    /* Always 0 */
	uint32_t  BitmapOffset; /* Starting position of image data in bytes */

    uint32_t Size;            /* Size of this header in bytes */
	int32_t  Width;           /* Image width in pixels */
	int32_t  Height;          /* Image height in pixels */
	uint16_t  Planes;          /* Number of color planes */
	uint16_t  BitsPerPixel;    /* Number of bits per pixel */
	/* Fields added for Windows 3.x follow this line */
	uint32_t Compression;     /* Compression methods used */
	uint32_t SizeOfBitmap;    /* Size of bitmap in bytes */
	int32_t  HorzResolution;  /* Horizontal resolution in pixels per meter */
	int32_t  VertResolution;  /* Vertical resolution in pixels per meter */
	uint32_t ColorsUsed;      /* Number of colors in the image */
	uint32_t ColorsImportant; /* Minimum number of important colors */
};


struct bitmap_header4
{
	uint32_t Size;            /* Size of this header in bytes */
	int32_t  Width;           /* Image width in pixels */
	int32_t  Height;          /* Image height in pixels */
	uint16_t  Planes;          /* Number of color planes */
	uint16_t  BitsPerPixel;    /* Number of bits per pixel */
	uint32_t Compression;     /* Compression methods used */
	uint32_t SizeOfBitmap;    /* Size of bitmap in bytes */
	int32_t  HorzResolution;  /* Horizontal resolution in pixels per meter */
	int32_t  VertResolution;  /* Vertical resolution in pixels per meter */
	uint32_t ColorsUsed;      /* Number of colors in the image */
	uint32_t ColorsImportant; /* Minimum number of important colors */
	/* Fields added for Windows 4.x follow this line */

	uint32_t RedMask;       /* Mask identifying bits of red component */
	uint32_t GreenMask;     /* Mask identifying bits of green component */
	uint32_t BlueMask;      /* Mask identifying bits of blue component */
	uint32_t AlphaMask;     /* Mask identifying bits of alpha component */
	uint32_t CSType;        /* Color space type */
	int32_t  RedX;          /* X coordinate of red endpoint */
	int32_t  RedY;          /* Y coordinate of red endpoint */
	int32_t  RedZ;          /* Z coordinate of red endpoint */
	int32_t  GreenX;        /* X coordinate of green endpoint */
	int32_t  GreenY;        /* Y coordinate of green endpoint */
	int32_t  GreenZ;        /* Z coordinate of green endpoint */
	int32_t  BlueX;         /* X coordinate of blue endpoint */
	int32_t  BlueY;         /* Y coordinate of blue endpoint */
	int32_t  BlueZ;         /* Z coordinate of blue endpoint */
	uint32_t GammaRed;      /* Gamma red coordinate scale value */
	uint32_t GammaGreen;    /* Gamma green coordinate scale value */
	uint32_t GammaBlue;     /* Gamma blue coordinate scale value */
};
#pragma pack(pop)


struct vec3{
	float x, y, z;
	vec3& operator-=(const vec3& other){
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		return *this;
	}
	vec3& operator+=(const vec3& other){
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		return *this;
	}
	vec3& operator*=(float value){
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}
	vec3& operator/=(float value){
		ASSERT(value != 0.0f);
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
};
vec3 operator-(vec3 left, const vec3& right){
	left -= right;
	return left;
}
vec3 operator+(vec3 left, const vec3& right){
	left += right;
	return left;
}
vec3 operator*(vec3 left, float value){
	left *= value;
	return left;
}
vec3 operator/(vec3 left, float value){
	left /= value;
	return left;
}

float dot(vec3 left, const vec3& right){
	return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);  
}

std::ostream& operator<<(std::ostream& stream, const vec3& v){
	std::cout << "{ " << v.x << ", " << v.y << ", " << v.z << "}";
	return stream;
}
float length2(const vec3& v){
	return v.x*v.x + v.y*v.y + v.z*v.z;
}
float length(const vec3& v){
	return sqrtf(length2(v));
}

struct Plane{
	vec3 normal;
	float distance;
};

struct Sphere{
	float radius;
	vec3 position;
};

uint32_t FloatRGBToPixel(const vec3& color, float alpha){
	return (uint8_t)(255 * alpha) << 24| (uint8_t)(255 * color.x) << 16|(uint8_t)(255 * color.y) << 8|(uint8_t)(255 * color.z);
}


void Intersect(const vec3& rayOrigin, const vec3& rayDir, const Plane& plane, uint32_t& color){
	// float d = dot(plane.normal, point);
	// return d - plane.distance > 0;
	float d = plane.distance * length(plane.normal);
	float t = (d - dot(rayOrigin, plane.normal)) / dot(plane.normal, rayDir);
	if (t > 0){
		color = FloatRGBToPixel({0.18f, 0.31f, 0.62f}, 1.f);
	}
}
void Intersect(const vec3& rayOrigin,const vec3& rayDir, const Sphere& sphere, uint32_t& color){

	vec3 nextRayOrigin = rayOrigin;
	vec3 nextRayDir = rayDir;

	for(int i = 0;i < 1; i++){
		float a = dot(nextRayDir, nextRayDir);
		float b = 2 * (dot(nextRayDir, nextRayOrigin));
		float c = dot(nextRayOrigin, nextRayOrigin) - sphere.radius*sphere.radius;

		float disc = (b*b) - (4*a*c);
		float div = 2 * a;
		if(disc < 0){
			return;
		}
		float threshold = 0.001f;
		if (abs(disc) < threshold){
			return;
		}
		float disc_div = sqrtf(disc) / div;
		float t1 = -2*b + disc_div;
		float t2 = -2*b - disc_div; 
		float t = (length(nextRayDir*t1 - nextRayOrigin) < length(nextRayDir*t1 - nextRayOrigin)) ? t1 : t2;

		nextRayDir = ((nextRayOrigin + nextRayDir*t) - sphere.position);
		nextRayOrigin = (nextRayOrigin + nextRayDir*t);
	}
	
	color = FloatRGBToPixel({0.65f, 0.4f, 0.08f}, 1.f);
}


void SetPixel(uint32_t* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t pixel){
	ASSERT((y * width + x) < (width * height));
	*(data + (y * width + x)) = pixel;
}

int main(){

    uint32_t width = 800;
    uint32_t height = 800;

    uint32_t* data = new(uint32_t[width * height * sizeof(uint32_t)]);


    uint32_t imageSize = width * height * sizeof(uint32_t);

    bitmap_header header = {};

    header.FileType = 0x4d42;     /* File type, always 4D42h ("BM") */
	header.FileSize = sizeof(bitmap_header) + imageSize;     /* Size of the file in bytes */
	header.BitmapOffset = sizeof(bitmap_header); /* Starting position of image data in bytes */

    header.Size = sizeof(bitmap_header) - 14;            /* Size of this header in bytes */
	header.Width = width;           /* Image width in pixels */
	header.Height = height;          /* Image height in pixels */
	header.Planes = 1;          /* Number of color planes */
	header.BitsPerPixel = 32;    /* Number of bits per pixel */
	header.SizeOfBitmap = imageSize;    /* Size of bitmap in bytes */


	vec3 sensorPositon = {0.f, 0.8f, 5.f};
	vec3 screenPosition = {sensorPositon.x, sensorPositon.y, sensorPositon.z - 1.f};

	Plane plane;
	plane.normal = {0.f, 1.f, 0.f};
	plane.distance = 0;
	Sphere sphere;
	sphere.position = {0.f, 0.f, 0.f};
	sphere.radius = 1.f; 

	float x_offset = (1.f/(width*2));
	float y_offset = (1.f/(height*2));
	for(int x = 0; x < width; x++){
		for(int y = 0; y < height; y++){
			
			float s_x = x/(float)width - 0.5f + x_offset;
			float s_y = (1-y/(float)height) - 0.5f + y_offset;

			vec3 rayDir = (screenPosition + vec3{s_x, s_y, 0.f}) - sensorPositon;

			float theta = -10/180.f * 3.1415926;
			float _y = rayDir.y;
			float _z = rayDir.z;
			rayDir.y = _y*cosf(theta) - _z*sin(theta);
			rayDir.z = _z*cosf(theta) + _y*sin(theta);

			uint32_t pixel = FloatRGBToPixel({0.2f, 0.2f, 0.2f}, 1.f);
			Intersect(sensorPositon, rayDir, plane, pixel);
			Intersect(sensorPositon, rayDir, sphere, pixel);
			SetPixel(data, x, ((width-1) - y), width, height, pixel);
		}
	}

    FILE* out_file = fopen("render.bmp", "wb");

    fwrite(&header, 1, sizeof(bitmap_header), out_file);
    fwrite(data, 1, imageSize, out_file);
    fclose(out_file);

}
