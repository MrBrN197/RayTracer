#include <iostream>

#include "math_functions.h"


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


struct Plane{
	vec3 normal;
	float distance;
	uint32_t matIndex;
};

struct Sphere{
	vec3 position;
	float radius;
	uint32_t matIndex;
};

struct Material{
	float roughness;
	vec3 specularColor;
	vec3 emitColor;
};

struct World{
	Plane* planes;
	Sphere* spheres;
	Material* materials;
	uint32_t numSpheres;
	uint32_t numPlanes;
	uint32_t numMaterials;
};



uint32_t FloatRGBToPixel(const vec3& color, float alpha){
	return (uint8_t)(255 * alpha) << 24| (uint8_t)(255 * color.x) << 16|(uint8_t)(255 * color.y) << 8|(uint8_t)(255 * color.z);
}

float Clamp(float value){
	if (value > 1.f)
		value = 1.f;
	if (value < 0.f)
		value = 0.f;
	return value;
}

vec3 Clamp(vec3 v){
	v.x = Clamp(v.x);
	v.y = Clamp(v.y);
	v.z = Clamp(v.z);
	return v;
}


vec3 RayCast(vec3 rayOrigin, vec3 rayDir, World* world){
	float threshold = 0.01f;
	vec3 result = {};

	vec3 specular = {1, 1, 1};

	vec3 nextRayOrigin = {};
	vec3 nextNormal = {};
	
	for(uint32_t rayCount = 0; rayCount < 4; rayCount++){
		float maxDistance = FLT_MAX;
		uint32_t HitMatIndex = 0;

		for(uint32_t i = 0; i < world->numPlanes; i++){
			Plane plane = world->planes[i]; 
			float d = plane.distance * length(plane.normal);
			float t = (d - dot(plane.normal, rayOrigin))/dot(plane.normal, rayDir);

			if ((t > 0) && (t < maxDistance)){
				maxDistance = t;
				nextNormal = normalize(plane.normal);
				HitMatIndex = plane.matIndex;
			}
		}

		for(uint32_t i = 0; i < world->numSpheres; i++){
			Sphere sphere = world->spheres[i];
			
			vec3 relativeToOrigin = rayOrigin - sphere.position;

			float a = dot(rayDir, rayDir);
			float b = 2 * dot(rayDir, relativeToOrigin);
			float c = dot(relativeToOrigin, relativeToOrigin) - sphere.radius * sphere.radius;

			float disc = (b*b) - (4*a*c);
			float div = 2 * a;
			if(disc < 0){
				continue;
			}
			ASSERT(abs(div) > 0.00001f);
			float t1 = (-b + sqrtf(disc)) / div;
			float t2 = (-b - sqrtf(disc)) / div;

			float t = t1;
			if ((t2 > 0) && (t2 < t1)){
				t = t2;
			}

			if((t > threshold) && (t < maxDistance)){
				maxDistance = t;
				nextRayOrigin = rayOrigin + (rayDir*t);
				nextNormal = normalize(nextRayOrigin - sphere.position);

				float distance = length(nextRayOrigin - sphere.position);
				ASSERT((distance - sphere.radius) < 0.0001f);
				HitMatIndex = sphere.matIndex;
			}
		}

		Material material = world->materials[HitMatIndex];
		if (HitMatIndex){
			result += ComponentwiseMultiply(specular, material.emitColor);
			specular = ComponentwiseMultiply(specular, material.specularColor);

			// float phiRange = (float)PI * material.roughness;
			// CoordinateFrame coordinateFrame = CreateCoordinatFrame(reflectionDir);

			// float anglePhi = (float)PI/2.f - phiRange/2.f + RandomUnilateral() * phiRange;
			// float angleTheta = RandomUnilateral() * 2*(float)PI;

			// float x = cosf(anglePhi) * cosf(angleTheta);
			// float y = cosf(anglePhi) * sinf(angleTheta);
			// float z = sinf(anglePhi);

			// rayDir = normalize(coordinateFrame.lateral * x + coordinateFrame.up * y + coordinateFrame.front * z);
			// rayDir = reflectionDir;
			rayOrigin += maxDistance*rayDir;

			vec3 reflectionDir = rayDir - 2 * dot(rayDir, nextNormal) * nextNormal;
			vec3 randomBounce = nextNormal + vec3{RandomUnilateral()*2.f - 1.f, RandomUnilateral()*2.f - 1.f, RandomUnilateral()*2.f - 1.f};
			rayDir = normalize(LinearInterpolate(reflectionDir, randomBounce, material.roughness));
			// rayDir = normalize(reflectionDir);
		}else{
			result += ComponentwiseMultiply(specular, material.emitColor);
			break;
		}
	}

	result = Clamp(result);
	return result;
}



void SetPixel(uint32_t* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t pixel){
	ASSERT((y * width + x) < (width * height));
	*(data + (y * width + x)) = pixel;
}

int main(){

    uint32_t width = 1280;
    uint32_t height = 720;

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


	Plane planes[1] = {
		{{0.f, 1.f, 0.f}, 0.f, 1}
	};

	Sphere spheres[3] = {
		{{0.f,  0.f,  0.f}, 1.0f, 2},	// 0
		{{-2.f, 1.f, 0.f}, 1.0f, 3},	// 1
		{{2.f,  0.5f, 1.f}, 0.5f, 5}	// 2
		// spheres[3].position = {-0.5f, 1.f, -5.f};
		// spheres[3].radius = 1.5f;
		// spheres[3].matIndex = 5;
		// spheres[4].position = {4.f, 0.f, 5.f};
		// spheres[4].radius = 0.4f;
		// spheres[4].matIndex = 6;
	};
	Material materials[7] = {
		{     0, {                }, { 0.3f,  0.4f,  0.5f}},	// 0
		{  0.3f, {0.5f, 0.5f, 0.5f}, {					 }},	// 1
		{ 0.95f, {0.7f, 0.5f, 0.3f}, {					 }},	// 2
		{  0.1f, { 1.f,  1.f,  1.f}, { 1.f,   1.f,    1.f}},	// 3
		{0.001f, {0.2f, 0.2f, 0.2f}, {0.24f,  0.3f,  0.9f}},	// 4
		{  0.6f, {0.1f, 0.8f, 0.3f}, {				     }},	// 5
		{  0.1f, {                }, {  1.f,  0.5f,  1.0f}}		// 6
	};
	
	World world;
	world.numMaterials = sizeof(materials)/sizeof(materials[0]);
	world.numPlanes = sizeof(planes)/sizeof(planes[0]);
	world.numSpheres = sizeof(spheres)/sizeof(spheres[0]);
	world.materials = materials;
	world.spheres = spheres;
	world.planes = planes;

	uint32_t numSamples = 32;
	float contribution = 1/(float)numSamples;


	vec3 cameraP = {0.0f, 1.0f, 5.0f};
	CoordinateFrame Camera = CreateCoordinatFrame(normalize(vec3{0.0f, 0.0f, 0.0f} - cameraP));
	Camera.lateral *= -1.0f;

	float FilmDist = 1.0f;
	float filmW = 1.0f * width/(float)height;
	float filmH = 1.0f;
	float halfFilmW = 0.5f*filmW;
	float halfFilmH = 0.5f*filmH;

	float sampleOffsetX = filmW/width;
	float sampleOffsetY = filmH/height;


	vec3 filmCenter = cameraP + FilmDist * Camera.front;
	for(uint32_t x = 0; x < width; x++){
		float filmX = (x/(float)width - 0.5f) * 2.0f;
		for(uint32_t y = 0; y < height; y++){
			float filmY = ((1-y/(float)height) - 0.5f) * 2.0f;

			vec3 finalColor = {};

			for(uint32_t sample = 0; sample < numSamples; sample++){
				float s_x = filmX + RandomUnilateral() * sampleOffsetX;
				float s_y = filmY + RandomUnilateral() * sampleOffsetY;

				vec3 filmP = filmCenter + (halfFilmW * Camera.lateral * s_x) + (Camera.up * s_y * halfFilmH);
				vec3 rayOrigin = cameraP;
				vec3 rayDirection = normalize(filmP - cameraP);  
				finalColor += contribution * RayCast(rayOrigin, rayDirection, &world);
			}

			uint32_t pixel = FloatRGBToPixel(finalColor, 1.f);
			SetPixel(data, x, ((height-1) - y), width, height, pixel);
		}
		if (x % (width/10) == 0){
			printf("Progress: %d%%\n", 100 * x/width);
		}
	}

    FILE* out_file = fopen("render.bmp", "wb");

    fwrite(&header, 1, sizeof(bitmap_header), out_file);
    fwrite(data, 1, imageSize, out_file);
    fclose(out_file);

}
