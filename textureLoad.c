/*************************************************************************
 * The functions for reading in 256 X 256 X 3 RLE encoded SGI RGB files  *
 *************************************************************************/

void open_image_file(char *, unsigned char [256][256][4]);
void decode_texel_line(unsigned char *, unsigned char [256][256][4], int, int);

void open_image_file(char *fileName, unsigned char out_bytes[256][256][4]){
  FILE *inFile;
  int i, j, sum;
  int out_pos, in_pos;
  unsigned char tmp, count;

  short imagic;
  char storage;
  char bpc;
  unsigned short dim;
  unsigned short sizeX, sizeY, sizeZ;

  unsigned char in_bytes[1024];
  int offset_table[768][2];

  /* open file and read type details out of header */
  inFile = fopen(fileName, "rb");
  if(inFile==NULL){
    fprintf(stdout, "Can not open file %s\n", fileName);
    exit(1);
  }
  fread(in_bytes, 1, 12, inFile);

  /* convert type details */
  imagic = (in_bytes[0]<<8) + in_bytes[1];
  storage = in_bytes[2];
  bpc = in_bytes[3];
  dim = (in_bytes[4]<<8) + in_bytes[5];
  sizeX = (in_bytes[6]<<8) + in_bytes[7];
  sizeY = (in_bytes[8]<<8) + in_bytes[9];
  sizeZ = (in_bytes[10]<<8) + in_bytes[11];

  /* test for correct file type */

  if(imagic!=474){
    printf("Bad SGI magic number: %d\n", imagic);
    exit(0);
  }

  if(storage!=1){
    printf("Storage is not run length encoded.\n");
    exit(0);
  }

  if(bpc!=1){
    printf("Bad number of bytes per pixel: %d\n", bpc);
    exit(0);
  }

  if(dim!=3){
    printf("Bad dimension: %d\n", dim);
    exit(0);
  }

  if(sizeX!=256 || sizeY!=256 || sizeZ!=3){
    printf("Bad file size: X is %d, Y is %d, Z is %d\n\n", sizeX, sizeY, sizeZ);
    exit(0);
  }

  /* skip remains of header and move to start of offset table */
  fseek(inFile, 512, SEEK_SET);

  /**************************************
   * read the table of offsets to lines *
   **************************************/
  for(i=0; i<768; i++){
    fread(in_bytes, 1, 4, inFile);
    offset_table[i][0] =  (in_bytes[0]<<24) + (in_bytes[1]<<16) + (in_bytes[2]<<8) + in_bytes[3];
  }

  for(i=0; i<768; i++){
    fread(in_bytes, 1, 4, inFile);
    offset_table[i][1] =  (in_bytes[0]<<24) + (in_bytes[1]<<16) + (in_bytes[2]<<8) + in_bytes[3];
  }

  /* loop for each line */
  for(i=0; i<256; i++){
    /* find the red line */
    fseek(inFile,  offset_table[i][0], SEEK_SET);

    /* read the red line */
    fread(in_bytes, 1, offset_table[i][1], inFile);

    /* decode the red line */
    decode_texel_line(in_bytes, out_bytes, i, 0);

    /* repeat for blue and green */
    fseek(inFile,  offset_table[i+256][0], SEEK_SET);
    fread(in_bytes, 1, offset_table[i+256][1], inFile);
    decode_texel_line(in_bytes, out_bytes, i, 1);

    fseek(inFile,  offset_table[i+512][0], SEEK_SET);
    fread(in_bytes, 1, offset_table[i+512][1], inFile);
    decode_texel_line(in_bytes, out_bytes, i, 2);

    /* set alpha to one */
    for(j=0; j<256; j++)
      out_bytes[i][j][3] = 255;
  }

  return;
}

/****************************************************
 * IF highes order bit of first byte is 1 the next  *
 * 7 bits specify the number of bytes to read       *
 * ELSE the next 7 bits specify the number of times *
 * the next byte must be repeated                   *
 * loop ends when 7 bits are zero                   *
 ****************************************************/
void decode_texel_line(unsigned char in_bytes[], unsigned char out_bytes[256][256][4], int line, int colour){
  int j;
  int out_pos=0;
  int in_pos=1;
  unsigned char tmp = in_bytes[0] & 0x80;
  unsigned char count = in_bytes[0] & 0x7f;

  while(count){
    if(tmp){
      for(j=0; j<count; j++)
        out_bytes[line][out_pos+j][colour] = in_bytes[in_pos+j];
      out_pos += count;
      in_pos  += count;
    }else{
      for(j=0; j<count; j++)
        out_bytes[line][out_pos+j][colour] = in_bytes[in_pos];
      out_pos += count;
      in_pos++;
    }
    count = in_bytes[in_pos] & 0x7f;
    tmp = in_bytes[in_pos] & 0x80;
    in_pos++;
  }
  return;
}
