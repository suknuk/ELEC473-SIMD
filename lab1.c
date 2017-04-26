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
	/*
	int input = 1,length=1,output;
	__asm__(
		"mov %[in], %%esi\n"
		"mov %[l], %%ecx\n"
	"l1:\n"
		"movdqu (%%esi),%%xmm7\n"
		"add $16,%%esi\n"
		"add $16,%%ecx\n"
	"jnz l1\n"
		:"=m"(input),"=m" (output)								//outputs
		:[in]"m" (input), [l]"m" (length),[out]"m" (output):	//inputs
		"esi", "ecx", "xmm7"									//clobbers
		);
	*/
	
	/*
	int src = 1;
	int dst;
	
	__asm__(
		"mov %1, %0\n\t"
		"add $1, %0"
		: "=r" (dst)	// output
		: "r" (src)		// input
		);
	printf("%d\n", dst);
	*/
	
	/*
	ALGO:
	index=0
	loop:
		if img data > threshold 
			apply threshold
		
		move to next data index
		
		index++
		if index < img->size
			goto loop
	*/
	
	unsigned char* img_new = (unsigned char*)malloc(image->size);
	
	int out1 = 0,out2 = 0,input1 = 10;
	printf("%d %d %d\n", out1, out2, image->size);
	
	
	__asm__ (
		"mov $0, %%ecx\n\t"
	"l1:\n\t"
		"add $3, %%ecx\n\t"
		"sub $1, %%eax\n\t"
		"cmpl $0, %%eax\n\t"
		"jne l1\n\t"
		
		// outputs
		:"=c" (out1)	
		// inputs
		:"a" (image->size),	//image size on eax
		"b" (input1)		// image data on ebx
		
	);
	
	printf("%d %d %d\n", out1, out2, image->size);
	
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
