internal void
LoadImageFromimage_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&image_h, sizeof(image_h));
image->x = image_hx;
image->y = image_hy;
image->n = image_hn;
}
