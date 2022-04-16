internal void
LoadImageFromgrass2_h(Image* image)
{
image->data = (unsigned char *)qalloc((void*)&grass2_h, sizeof(grass2_h));
image->x = grass2_hx;
image->y = grass2_hy;
image->n = grass2_hn;
}
