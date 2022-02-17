#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

internal void
SaveImageToHeaderFile(char* filename, Image* image)
{
    char* fullDir = StringConcat("imagesaves/", filename);
    
    // Header file
    FILE *newfile = fopen(fullDir, "w");
    
    char f[sizeof(filename)];
    FilenameSearchModify(filename, f);
    char fcapital[sizeof(filename)];
    FilenameCapitalize(filename, fcapital);
    
    fprintf(newfile,
            "#ifndef %s\n"
            "#define %s\n"
            "int %sx = %d;\n"
            "int %sy = %d;\n"
            "int %sn = %d;\n"
            "static unsigned char %s[%d] = \n"
            "{\n"
            ,fcapital
            ,fcapital
            ,f
            ,image->x
            ,f
            ,image->y
            ,f
            ,image->n
            ,f
            ,(image->x * image->y * image->n));
    
    
    unsigned char* imgtosave = image->data;
    for(int i = 0; i < (image->x * image->y * image->n); i++)
    {
        fprintf(newfile,
                "0x%x ,",
                *imgtosave);
        *imgtosave++;
    }
    fprintf(newfile, 
            "};\n"
            "\n"
            "#endif");
    fclose(newfile);
    
    // C++ file
    char* filenamecpp = (char*)PermanentStorageAssign(fullDir, StringLength(fullDir) + 2);
    
    int j = 0;
    int extension = 0;
    char* cursor = fullDir;
    while (!extension)
    {
        if (*cursor == '.')
        {
            filenamecpp[j] = *cursor;
            extension = 1;
        }
        else
        {
            filenamecpp[j] = *cursor;
        }
        
        cursor++;
        j++;
    }
    filenamecpp[j] = 'c';
    filenamecpp[j + 1] = 'p';
    filenamecpp[j + 2] = 'p';
    filenamecpp[j + 3] = 0;
    
    FILE *newcppfile = fopen(filenamecpp, "w");
    fprintf(newcppfile, 
            "internal void\n"
            "LoadImageFrom%s(Image* image)\n"
            "{\n"
            "image->data = (unsigned char *)PermanentStorageAssign((void*)&%s, sizeof(%s));\n"
            "image->x = %sx;\n"
            "image->y = %sy;\n"
            "image->n = %sn;\n"
            "}\n"
            ,f
            ,f
            ,f
            ,f
            ,f
            ,f
            );
    
    fclose(newcppfile);
    
}

internal Image
LoadImageResize(char* FileName, int Width, int Height, char* SaveFileName)
{
    // Set working directory in visual studio to the image save folder
    Image NewImage;
    NewImage.data = stbi_load(FileName, &NewImage.x, &NewImage.y, &NewImage.n, 0);
    
    Assert(NewImage.data != 0); // Image loaded
    
    Rect imageRect = {0, 0, Width, Height, 0};
    unsigned char* resized = (unsigned char *)PermanentStorageBlank(imageRect.width * imageRect.height * NewImage.n);
    
    stbir_resize_uint8(NewImage.data , NewImage.x, NewImage.y, 0,
                       resized, imageRect.width, imageRect.height, 0, NewImage.n);
    
    unsigned char* toBeFreed = NewImage.data;
    
    NewImage.data = resized;
    NewImage.x = Width;
    NewImage.y = Height;
    
    SaveImageToHeaderFile(SaveFileName, &NewImage);
    stbi_image_free(toBeFreed);
    
    return NewImage;
}