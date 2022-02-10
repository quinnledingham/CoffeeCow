internal void
LoadImageFromgrass_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&grass_h, sizeof(grass_h));
image->x = grass_hx;
image->y = grass_hy;
image->n = grass_hn;
}
