#include "Texture.h"

using namespace Eigen;

Texture::Texture(std::string& filename, DecalMode dm, Interpolation interpolation, TextureType type, int normalizer, float bumpFactor){
    if (IsPNG(filename.c_str())){
        isPng = true;
        rowPointers = nullptr;
        ReadPNG(filename.c_str());
    }
    else{
        isPng = false;
        ReadJPG(filename.c_str());
    }

    this->decalMode = dm;
    this->interpolation = interpolation;
    this->normalizer = normalizer;
    this->type = type;
    this->bumpFactor = bumpFactor;
}

Texture::Texture(DecalMode dm, Interpolation interpolation, TextureType type, NoiseConversion nc, int normalizer, float noiseScale, float bumpFactor){
    this->decalMode = dm;
    this->interpolation = interpolation;
    this->normalizer = normalizer;
    this->type = type;
    this->nc = nc;
    this->noiseScale = noiseScale;
    this->bumpFactor = bumpFactor;
}

Eigen::Vector3f Texture::GetColorAtPixel(int i, int j) {
    if (i < 0){
        i = 0;
    }
    else if (i >= width){
        i = width-1;
    }
    if (j < 0){
        j = 0;
    }
    else if (j >= height){
        j = height-1;
    }

    if (isPng){
        Vector3f color = {(float)(&rowPointers[j][i*4])[0], (float)(&rowPointers[j][i*4])[1],
                         (float)(&rowPointers[j][i*4])[2]};
        return color;
        /*return Vector3f{(float)(&rowPointers[j][i*4])[0], (float)(&rowPointers[j][i*4])[1],
                        (float)(&rowPointers[j][i*4])[2]};*/
    }
    else{
        Vector3f color = {(float)rawImage[(j*width+i)*numComponents], (float)rawImage[(j*width+i)*numComponents + 1],
                         (float)rawImage[(j*width+i)*numComponents + 2]};
        return color;
        /*return Vector3f{(float)rawImage[(j*width+i)*numComponents], (float)rawImage[(j*width+i)*numComponents + 1],
                        (float)rawImage[(j*width+i)*numComponents + 2]};*/
    }
}

Vector2f Texture::GetChangeAtCoordinates(float u, float v) {
    u = u - floor(u);
    v = v - floor(v);
    int i = u * width;
    int j = v * height;

    if (i < 0){
        i = 0;
    }
    else if (i >= width-1){
        i = width-2;
    }
    if (j < 0){
        j = 0;
    }
    else if (j >= height-1){
        j = height-2;
    }

    float dede = (GetColorAtPixel(i+1,j)[0] + GetColorAtPixel(i+1,j)[1] + GetColorAtPixel(i+1,j)[2]) / 3.0f;
    float nene = (GetColorAtPixel(i,j)[0] + GetColorAtPixel(i,j)[1] + GetColorAtPixel(i,j)[2]) / 3.0f;
    Vector3f du = GetColorAtPixel(i+1,j) - GetColorAtPixel(i,j);
    Vector3f dv = GetColorAtPixel(i,j+1) - GetColorAtPixel(i,j);
    //std::cout << "i: " << i << " j: " << j << std::endl;
    //std::cout << GetColorAtPixel(i+1,j)[0] << ", " << GetColorAtPixel(i+1,j)[1] << ", " << GetColorAtPixel(i+1,j)[2] << std::endl;
    //std::cout << GetColorAtPixel(i,j)[0] << ", " << GetColorAtPixel(i,j)[1] << ", " << GetColorAtPixel(i,j)[2] << std::endl;

    //float uDerivative = (du[0] + du[1] + du[2]) / 3.0f;
    float uDerivative = (dede - nene);
    float vDerivative = (dv[0] + dv[1] + dv[2]) / 3.0f;
    //vDerivative = vDerivative / 255;

    return Vector2f{uDerivative, vDerivative};
}

Eigen::Vector3f Texture::GetColorAtCoordinates(float u, float v) {
    u = u - floor(u);
    v = v - floor(v);
    float i = u * width;
    float j = v * height;

    if (interpolation == NN){
        return GetColorAtPixel((int)i, (int)j);
    }
    else{
        int leftTop_i = floor(i);
        int leftTop_j = floor(j);
        float a = i - leftTop_i;
        float b = j - leftTop_j;

        return (1-a)*(1-b)*GetColorAtPixel(leftTop_i, leftTop_j)
        + (1-a)*b*GetColorAtPixel(leftTop_i, leftTop_j+1)
        + a*(1-b)*GetColorAtPixel(leftTop_i+1, leftTop_j)
        + a*b*GetColorAtPixel(leftTop_i+1, leftTop_j+1);
    }
}

bool Texture::IsPNG(const char *filename) {
    int i = 0;
    int c = 0;
    while (filename[i] != '\0'){
        if (filename[i] == '.'){
            c = 1;
        }
        else if (filename[i] == 'p' && c == 1){
            c = 2;
        }
        else if (filename[i] == 'n' && c == 2){
            c = 3;
        }
        else if (filename[i] == 'g' && c == 3){
            return true;
        }
        else{
            c = 0;
        }

        i++;
    }

    return false;
}

void Texture::ReadJPG(const char *filename) {
    JSAMPROW row_pointer[1];

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE *infile = fopen( filename, "rb" );
    unsigned long location = 0;
    int i = 0, j = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file: %s\n", filename );
        return;
    }

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error( &jerr );
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress( &cinfo );
    /* this makes the library read from infile */
    jpeg_stdio_src( &cinfo, infile );
    /* reading the image header which contains image information */
    jpeg_read_header( &cinfo, TRUE );
    /* Start decompression jpeg here */
    jpeg_start_decompress( &cinfo );

    width = cinfo.image_width;
    height = cinfo.image_height;
    numComponents = cinfo.num_components;

    /* allocate memory to hold the uncompressed image */
    rawImage = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
    /* read one scan line at a time */
    while( cinfo.output_scanline < cinfo.image_height )
    {
        jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
            rawImage[location++] = row_pointer[0][i];
    }

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    fclose( infile );
}

void Texture::ReadPNG(const char *filename) {
    FILE *fp = fopen(filename, "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(bit_depth == 16)
        png_set_strip_16(png);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (rowPointers) abort();

    rowPointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        rowPointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }

    png_read_image(png, rowPointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);
}