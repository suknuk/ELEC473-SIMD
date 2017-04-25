#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h> // strcmp

// struct to hold data for the image
struct Image
{
	unsigned char* data;
	size_t size;
};

// enum to define which processing type should be used
enum PROCESSING_TYPE {C, SIMD};

// reading an image file to an Image struct
int open_image(const char* path, struct Image* image)
{
	FILE* input = fopen(path, "rb"); //rb=read binary
	
	if (input == NULL){
		printf("Error: Reading %s is null\n", path);
		return 11;
	}

	// get size of input file
	fseek(input, 0, SEEK_END);
	image->size = ftell(input);
	fseek(input, 0, SEEK_SET);	

	// reserver memory and read data into the struct
	image->data = (unsigned char*)malloc(image->size);
	fread(image->data, sizeof(unsigned char), image->size, input);

	fclose(input);
	return 0;	
} 

// function to save the modified image
int save_image(const char* path, struct Image* image)
{
	FILE* output = fopen(path, "wb");//wb=write binary
	
	if (output == NULL) {
		printf("Error: Writing %s is null\n", path);
		return 12;
	}
	fwrite(image->data, sizeof(unsigned char), image->size, output);
	return 0;
}

// processing the threshold using c
void process_threshold_c(struct Image* image, int threshold)
{
	// sets pixel to black or white, depending if it is under/over
	// the threshold
	for (size_t i = 0; i < image->size; i++) {
		image->data[i] = image->data[i] > threshold ? 255 : 0;
	}
}

void process_threshold_SIMD(struct Image* image, int threshold)
{
	int src = 1;
	int dst;
	/*
	asm("mov %1, %0\n\t"
		"add $1, %0"
		: "=r" (dst)
		: "r" (src));
	printf("%d\n", dst);
	*/
	_asm{
		emms;
	}
}

//input threshold, image_path, .. , image_path
int main(int argc, char* argv[])
{
	if(argc < 4) {
		printf("Need at least 3 arguments\n");
		return 1;
	}

	int images_nr = argc - 3;
	int threshold;
	sscanf(argv[1],"%d",&threshold);
	printf("Applying threshold of %d to %d images\n",threshold,images_nr);
	
	enum PROCESSING_TYPE myStrategy;
	if(strcmp(argv[2], "C") == 0) {
		myStrategy = C;
		printf("Using C implementation\n");
	} else if(strcmp(argv[2], "SIMD") == 0) {
		myStrategy = SIMD;
		printf("Using SIMD implementation\n");
	} else {
		printf("No processing strategy defined\n");
		return 1;
	}
	

	time_t start, end;
	float dt;
	// iterate every given image
	for (int i = 3; i < argc; i++) { 
		printf("iterating %s\n", argv[i]);
		struct Image image;
		// load image		
		int open = open_image(argv[i], &image);

		// process if no errors of opening image
		if (open == 0){
			start = clock();
			// iterate all pixels

			if (myStrategy == C){
				//process_threshold_c
				process_threshold_c(&image, threshold);
			} else {
				//SIMD
				process_threshold_SIMD(&image, threshold);
			}

			end = clock();
			dt = (end-start)/(float)(CLOCKS_PER_SEC);	

			printf("Computation time for %s : %f seconds\n",argv[i],dt);
			// save new img
			int save = save_image(argv[i], &image);
			if (save != 0) {
				printf("Error saving %s\n", argv[i]);
			}
		} 
	}
	return 0;
}	
