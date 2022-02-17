internal void
LoadImageFromapple_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&apple_h, sizeof(apple_h));
image->x = apple_hx;
image->y = apple_hy;
image->n = apple_hn;
}
