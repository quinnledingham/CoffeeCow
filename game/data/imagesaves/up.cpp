internal void
LoadImageFromup_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&up_h, sizeof(up_h));
image->x = up_hx;
image->y = up_hy;
image->n = up_hn;
}
