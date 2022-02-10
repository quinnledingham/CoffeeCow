internal void
LoadImageFromleft_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&left_h, sizeof(left_h));
image->x = left_hx;
image->y = left_hy;
image->n = left_hn;
}
