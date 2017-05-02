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
	for (size_t i = 0; i < image->size; i++) {
		__asm__
		(
			"cmp %%ebx, %%eax;"
			"jl larger;"
			"mov $0, %%edx;"
			"jmp end;"
		"larger:"
			"mov $255, %%edx;"
		"end:"
			//output
			:"=d"(image->data[i])
		 	//input
			:"a"(threshold),
			"b"(image->data[i])
		);
	}

	/*
	int ARR_LEN=10;
	int STEP_SIZE=1;
	long SOME_VALUE=100;

	//long* arr = new long[ARR_LEN];
	unsigned char* arr;
	unsigned char arrVal[ARR_LEN];
	arr = arrVal;

	int i;

	for (i=0; i<ARR_LEN; i++){
		arr[i] = i*2;
	}
	for (i=0; i<ARR_LEN; i++){
		printf("element %d is %u\n",i,arr[i]);
	}

	__asm__ __volatile__
	(
		"movq $3, (%%rbx);"
	"loop:"
		//"movq %%rdx, (%%rbx);"
		//"leaq (%%rbx, %%rcx, 1), %%rbx;"

		//"leaq (%%rbx, %%rcx,1), %%rbx;"
		"cmpq $10, %%rax;"
		"jg pos;"
		"jmp neg;"
	"pos:"
		"movq $255, (%%rbx);"
		"leaq (%%rbx, %%rcx,1), %%rbx;"
		"jmp loop_end;"
	"neg:"
		"movq $1, (%%rbx);"
		"leaq (%%rbx, %%rcx,1), %%rbx;"

	"loop_end:"
		"cmpq %%rbx, %%rax;"
		"jg loop;"
	
		: // no output
		: "b" (arr),
		"a" (arr+ARR_LEN),
		"c" (STEP_SIZE),
		"d" (SOME_VALUE)
		: "cc", "memory"
	);

	for (i=0; i<ARR_LEN; i++){
		printf("element %d is %u\n",i,arr[i]);
	}
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
	unsigned char* img_new = (unsigned char*)malloc(image->size);
	
	int out1 = 0,out2 = 0,input1 = 10;
	printf("%d %d %zu\n", out1, out2, image->size);
	
	__asm__ (
		"mov %%ebx, %%esi\n\t"
		"movdqu %%edi, %%xmm7\n\t"
	// loop start
	"loop1:\n\t"
		
	// check if point is higher or lower than the threshold

		"cmpl %%ecx, %%edx\n\t"		// compare threshold(ecx) with the img point(edx)
		"jg threshold_lower\n\t"	// threshold is lower -> jump to threshold_lower label
		"jmp threshold_higher\n\t"	// jump to threshold_higher otherwise
		
	"threshold_lower:\n\t"
		// data[i] = 0
		"jmp threshold_end\n\t"	

	"threshold_higher:\n\t"
		// data[i] = 255
		"jmp threshold_end\n\t"

	"threshold_end:\n\t"

	// loop image-size times
		"dec %%eax\n\t"		// decrease loop counter by one (image size)
		"cmpl $0, %%eax\n\t"	// compare it with 0
		"jne loop1\n\t"		// jump to 'loop1' if eax is not 0
		
		// outputs
		:"=d" (out1)
		
		// inputs
		:"a" (image->size),	//image size on eax
		"b" (image->data),	// image data on ebx
		"c" (threshold)		// threshold on ecx
		
	);
	
	printf("%d %d %zu\n", out1, out2, image->size);
	printf("%p\n", &image->data);
	*/
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
