internal void
LoadImageFromdown_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&down_h, sizeof(down_h));
image->x = down_hx;
image->y = down_hy;
image->n = down_hn;
}
