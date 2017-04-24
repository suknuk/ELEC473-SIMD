#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// struct to hold data for the image
struct Image
{
	unsigned char* data;
	size_t size;
};

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

//input threshold, image_path, .. , image_path
int main(int argc, char* argv[])
{
	int images_nr = argc - 2;
	int threshold;
	sscanf(argv[1],"%d",&threshold);
	printf("Applying threshold of %d to %d images\n",threshold,images_nr);

	time_t start, end;
	float dt;
	// iterate every given image
	for (int i = 1; i <= images_nr; i++) { 
		struct Image image;
		// load image		
		open_image(argv[i+1], &image);

		start = clock();
		// iterate all pixels

		end = clock();
		dt = (end-start)/(float)(CLOCKS_PER_SEC);	

		printf("Computation time for %s : %f seconds\n",argv[i+1],dt);
	}
//	for(int i = 0; i < argc; i++) {printf("%s\n", argv[i]); }
	return 0;
}	
