#include <iostream>
#include <Windows.h>

#include "math_functions.h"


#pragma pack(push, 1)
struct bitmap_header{
	uint16_t  FileType;     /* File type, always 4D42h ("BM") */
	uint32  FileSize;     /* Size of the file in bytes */
	uint16_t  Reserved1;    /* Always 0 */
	uint16_t  Reserved2;    /* Always 0 */
	uint32  BitmapOffset; /* Starting position of image data in bytes */

    uint32 Size;            /* Size of this header in bytes */
	int32  Width;           /* Image width in pixels */
	int32  Height;          /* Image height in pixels */
	uint16_t  Planes;          /* Number of color planes */
	uint16_t  BitsPerPixel;    /* Number of bits per pixel */
	/* Fields added for Windows 3.x follow this line */
	uint32 Compression;     /* Compression methods used */
	uint32 SizeOfBitmap;    /* Size of bitmap in bytes */
	int32  HorzResolution;  /* Horizontal resolution in pixels per meter */
	int32  VertResolution;  /* Vertical resolution in pixels per meter */
	uint32 ColorsUsed;      /* Number of colors in the image */
	uint32 ColorsImportant; /* Minimum number of important colors */
};


struct bitmap_header4
{
	uint32 Size;            /* Size of this header in bytes */
	int32  Width;           /* Image width in pixels */
	int32  Height;          /* Image height in pixels */
	uint16_t  Planes;          /* Number of color planes */
	uint16_t  BitsPerPixel;    /* Number of bits per pixel */
	uint32 Compression;     /* Compression methods used */
	uint32 SizeOfBitmap;    /* Size of bitmap in bytes */
	int32  HorzResolution;  /* Horizontal resolution in pixels per meter */
	int32  VertResolution;  /* Vertical resolution in pixels per meter */
	uint32 ColorsUsed;      /* Number of colors in the image */
	uint32 ColorsImportant; /* Minimum number of important colors */
	/* Fields added for Windows 4.x follow this line */

	uint32 RedMask;       /* Mask identifying bits of red component */
	uint32 GreenMask;     /* Mask identifying bits of green component */
	uint32 BlueMask;      /* Mask identifying bits of blue component */
	uint32 AlphaMask;     /* Mask identifying bits of alpha component */
	uint32 CSType;        /* Color space type */
	int32  RedX;          /* X coordinate of red endpoint */
	int32  RedY;          /* Y coordinate of red endpoint */
	int32  RedZ;          /* Z coordinate of red endpoint */
	int32  GreenX;        /* X coordinate of green endpoint */
	int32  GreenY;        /* Y coordinate of green endpoint */
	int32  GreenZ;        /* Z coordinate of green endpoint */
	int32  BlueX;         /* X coordinate of blue endpoint */
	int32  BlueY;         /* Y coordinate of blue endpoint */
	int32  BlueZ;         /* Z coordinate of blue endpoint */
	uint32 GammaRed;      /* Gamma red coordinate scale value */
	uint32 GammaGreen;    /* Gamma green coordinate scale value */
	uint32 GammaBlue;     /* Gamma blue coordinate scale value */
};
#pragma pack(pop)

struct Image{
	uint32 width;
	uint32 height;
	uint32* data;
};


struct Plane{
	vec3 normal;
	float distance;
	uint32 matIndex;
};

struct Sphere{
	vec3 position;
	float radius;
	uint32 matIndex;
};

struct Material{
	float roughness;
	vec3 specularColor;
	vec3 emitColor;
};

struct Scene{
	Plane* planes;
	Sphere* spheres;
	Material* materials;
	uint32 numSpheres;
	uint32 numPlanes;
	uint32 numMaterials;
};

struct Tile{
	Scene* scene;
	Image* image;
	uint32 xMin;
	uint32 yMin;
	uint32 tileWidth;
	uint32 tileHeight;	
};
struct RenderData{
	Tile *tiles;
    uint32 tileCount;
	uint32 finishedTiles;

    volatile uint32 nextTile;
};


uint32 FloatRGBToPixel(const vec3& color, float alpha){
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

uint32 LockedAdd(volatile uint32* value, uint32 addend){
    uint32 old = *value;
    *value += addend;
    return old;
}


vec3 RayCast(vec3 rayOrigin, vec3 rayDir, Scene* scene){
	float threshold = 0.01f;
	vec3 result = {};

	vec3 specular = {1, 1, 1};

	vec3 nextRayOrigin = {};
	vec3 nextNormal = {};
	
	for(uint32 rayCount = 0; rayCount < 32; rayCount++){
		float maxDistance = FLT_MAX;
		uint32 HitMatIndex = 0;

		for(uint32 i = 0; i < scene->numPlanes; i++){
			Plane plane = scene->planes[i]; 
			float d = plane.distance * length(plane.normal);
			float t = (d - dot(plane.normal, rayOrigin))/dot(plane.normal, rayDir);

			if ((t > 0) && (t < maxDistance)){
				maxDistance = t;
				nextNormal = normalize(plane.normal);
				HitMatIndex = plane.matIndex;
			}
		}

		for(uint32 i = 0; i < scene->numSpheres; i++){
			Sphere sphere = scene->spheres[i];
			
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

		Material material = scene->materials[HitMatIndex];
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



void SetPixel(Image* image, uint32 x, uint32 y, uint32 pixel){
	ASSERT((y * image->width + x) < (image->width * image->height));
	*(image->data + (y * image->width + x)) = pixel;
}
uint32* GetPixelPointer(Image* image, uint32 x, uint32 y){
	ASSERT((y * image->width + x) < (image->width * image->height));
	uint32 *ptr = (image->data + (y * image->width + x));
    return ptr;
}


bool RenderTile(RenderData* renderData){

    uint32_t nextTile = LockedAdd(&renderData->nextTile, 1);
    if(nextTile >= renderData->tileCount){
        return false;
    }

    Tile* tile = renderData->tiles + nextTile;

    Scene* scene = tile->scene;
    Image* image = tile->image;
    uint32 xMin = tile->xMin;
    uint32 yMin = tile->yMin;
    uint32 tileWidth = tile->tileWidth;
    uint32 tileHeight = tile->tileHeight;

	uint32 numSamples = 512;
	float contribution = 1/(float)numSamples;

	vec3 cameraP = {0.0f, 1.0f, 5.0f};
	CoordinateFrame Camera = CreateCoordinatFrame(normalize(cameraP));

	float FilmDist = 1.0f;
	float filmW = 1.0f * image->width/(float)image->height;
	float filmH = 1.0f;
	float halfFilmW = 0.5f*filmW;
	float halfFilmH = 0.5f*filmH;

	float sampleOffsetX = filmW/image->width;
	float sampleOffsetY = filmH/image->height;


	vec3 filmCenter = cameraP - FilmDist * Camera.front;
    for(uint32 y = yMin; y < (yMin + tileHeight); y++){
		if(y >= image->height)
			continue;

        float filmY = ((1-y/(float)image->height) - 0.5f) * 2.0f;
        uint32* ptr = GetPixelPointer(image, xMin, image->height - 1 - y);
		for(uint32 x = xMin; x < (xMin + tileWidth); x++){
			if(x >= image->width)
				continue;

            float filmX = (x/(float)image->width - 0.5f) * 2.0f;

			vec3 finalColor = {};
			for(uint32 sample = 0; sample < numSamples; sample++){
				float s_x = filmX + RandomUnilateral() * sampleOffsetX;
				float s_y = filmY - RandomUnilateral() * sampleOffsetY;

				vec3 filmP = filmCenter + (halfFilmW * Camera.lateral * s_x) + (Camera.up * s_y * halfFilmH);
				vec3 rayOrigin = cameraP;
				vec3 rayDirection = normalize(filmP - cameraP);  
				finalColor += contribution * RayCast(rayOrigin, rayDirection, scene);
			}

			uint32 pixel = FloatRGBToPixel(finalColor, 1.f);
            *ptr++ = pixel;
		}
	}
	return true;
}

DWORD WINAPI WorkerThread(LPVOID lpThreadParameter){
    RenderData* renderData = (RenderData*)lpThreadParameter;
    while(RenderTile(renderData)){}
    return 0;
}


int main(){

    uint32 width = 1280;
    uint32 height = 720;
	Image image;
	image.width = width;
	image.height = height;
	image.data = new(uint32[width * height * sizeof(uint32)]);


	Plane planes[1] = {
		{{0.f, 1.f, 0.f}, 0.f, 1}
	};

	Sphere spheres[4] = {
		{{ 0.0f,  0.f,  0.f}, 1.0f, 2},	// 0
		{{ -2.f,  1.f, 0.0f}, 1.0f, 3},	// 1
		{{  2.f, 0.5f, 1.0f}, 0.5f, 5},	// 2
		{{ 2.5f, 2.0f,-5.0f}, 1.5f, 4}   // 3
		// spheres[4].position = {4.f, 0.f, 5.f};
		// spheres[4].radius = 0.4f;
		// spheres[4].matIndex = 6;
	};
	Material materials[7] = {
		{     0, {                }, { 0.3f,  0.4f,  0.5f}},	// 0
		{  0.3f, {0.5f, 0.5f, 0.5f}, {					 }},	// 1
		{ 0.95f, {0.7f, 0.5f, 0.3f}, {					 }},	// 2
		{  0.1f, { 1.f,  1.f,  1.f}, { 1.f,   1.f,    1.f}},	// 3
		{  0.1f, {0.8f, 0.2f, 0.2f}, {                   }},	// 4
		{  0.6f, {0.1f, 0.8f, 0.3f}, {				     }},	// 5
		{  0.1f, {                }, {  1.f,  0.5f,  1.0f}}		// 6
	};
	
	Scene scene;
	scene.numMaterials = sizeof(materials)/sizeof(materials[0]);
	scene.numPlanes = sizeof(planes)/sizeof(planes[0]);
	scene.numSpheres = sizeof(spheres)/sizeof(spheres[0]);
	scene.materials = materials;
	scene.spheres = spheres;
	scene.planes = planes;

    uint32 threadCount = 32;
	uint32 tileCount = (width + threadCount - 1) / threadCount;
	uint32 tileWidth = (width + tileCount - 1) / tileCount;
	uint32 tileHeight = tileWidth;

	printf("TileCount: %d\n", tileCount);
	printf("TileWidth: %d TileHeight: %d\n", tileWidth, tileHeight);


    RenderData renderData = {};
    renderData.tiles = new Tile[tileCount*tileCount];

	for (uint32 y = 0; y < tileCount; y++){
		for (uint32 x = 0; x < tileCount; x++){
			uint32 xMin = x * tileWidth;
			uint32 yMin = y * tileHeight;

            Tile *tile = renderData.tiles + renderData.tileCount++;
            tile->image = &image;
            tile->scene = &scene;
            tile->tileHeight = tileHeight;
            tile->tileWidth = tileWidth;
            tile->xMin = xMin;
            tile->yMin = yMin;
		}
	}

    for(uint32 i = 0; i < threadCount-1; i++){
        LPDWORD threadId = nullptr;
        HANDLE threadHandle = CreateThread(NULL, NULL, WorkerThread, &renderData, NULL, threadId);
        ASSERT(threadHandle);
        CloseHandle(threadHandle);
    }

    while(true){
        if(RenderTile(&renderData)){
            uint32 percentage = (uint32)(100.0f * renderData.nextTile/((float)renderData.tileCount));
            printf("Progress: %d%%\n", percentage);
        }else{
            break;
        }
    }


    FILE* out_file = fopen("render.bmp", "wb");

	uint32 imageSize = width * height * sizeof(uint32);

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

    fwrite(&header, 1, sizeof(bitmap_header), out_file);
    fwrite(image.data, 1, imageSize, out_file);
    fclose(out_file);

}