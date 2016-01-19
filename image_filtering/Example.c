#include <math.h>
#include "tiff.h"
#include "allocate.h"
#include "randlib.h"
#include "typeutil.h"

#define FILTERH 9
#define FILTERW 9

void error(char *name);

int main (int argc, char **argv) 
{
	FILE *fp;
	struct TIFF_img input_img, output_img;
	int32_t i,j,m,n;
	/*int32_t pixel;*/
	double lpf[FILTERH][FILTERW];
	double rtemp, gtemp, btemp;

	for (i = 0; i < FILTERH; i++) {
		for (j = 0; j < FILTERW; j++) {
			lpf[i][j] = 1.0 / 81;
		}
	}

	if (argc != 2) error(argv[0]);

	/* open image file */
	if ((fp = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "cannot open file %s\n", argv[1]);
		exit(1);
	}

	/* read image */
	if (read_TIFF(fp, &input_img)) {
		fprintf(stderr, "error reading file %s\n", argv[1]);
		exit(1);
	}

	/* close image file */
	fclose(fp);

	/* check the type of image data */
	if (input_img.TIFF_type != 'c') {
		fprintf(stderr, "error:  image must be 24-bit color\n");
		exit(1);
	}

	/* set up structure for output color image */
	/* Note that the type is 'c' rather than 'g' */
	get_TIFF(&output_img, input_img.height, input_img.width, 'c');

	for (i = 0; i < input_img.height; i++) {
		for (j = 0; j < input_img.width; j++) {
			rtemp = 0.0;
			gtemp = 0.0;
			btemp = 0.0;
			for (m = 0; m < FILTERH; m++) {
				for (n = 0; n < FILTERW; n++) {
					if ((i-m) >= 0 && (j-n) >= 0
						&& (i-m) <= input_img.height
						&& (j-n) <= input_img.width) {
						rtemp += lpf[m][n] * input_img.color[0][i-m][j-n]; 
						gtemp += lpf[m][n] * input_img.color[1][i-m][j-n]; 
						btemp += lpf[m][n] * input_img.color[2][i-m][j-n]; 
					}	
				}
			}
		  	output_img.color[0][i][j] = (int)rtemp;
		  	output_img.color[1][i][j] = (int)gtemp;
		  	output_img.color[2][i][j] = (int)btemp;
		}
	}

	/* open color image file */
	if ((fp = fopen("output.tif", "wb")) == NULL) {
		fprintf(stderr, "cannot open file color.tif\n");
		exit(1);
	}

	/* write color image */
	if(write_TIFF(fp, &output_img)) {
		fprintf(stderr, "error writing TIFF file %s\n", argv[2]);
		exit(1);
	}

	/* close color image file */
	fclose(fp);

	/* de-allocate space which was used for the images */
	free_TIFF(&(input_img));
	free_TIFF(&(output_img));

	return(0);
}

void error(char *name)
{
    printf("usage:  %s  image.tiff \n\n",name);
    printf("this program reads in a 24-bit color TIFF image.\n");
    printf("It then horizontally filters the green component, adds noise,\n");
    printf("and writes out the result as an 8-bit image\n");
    printf("with the name 'green.tiff'.\n");
    printf("It also generates an 8-bit color image,\n");
    printf("that swaps red and green components from the input image");
    exit(1);
}