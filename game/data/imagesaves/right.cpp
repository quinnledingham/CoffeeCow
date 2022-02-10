internal void
LoadImageFromright_h(Image* image)
{
image->data = (unsigned char *)PermanentStorageAssign((void*)&right_h, sizeof(right_h));
image->x = right_hx;
image->y = right_hy;
image->n = right_hn;
}
