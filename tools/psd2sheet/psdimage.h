/*
  PSD image loader by DrPetter
  
  Work in progress, no guarantees but it seems to work.
  Use it for anything, but do so at your own risk.
  
  (published so that I can use this in LD48#11 without cheating)
*/

#ifndef psdimage_h
#define psdimage_h

#include <stdio.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

/*

usage
-----

PSDImage* img=new PSDImage();
if(!img->Load("image.psd")) // load image file and store image data for all layers
	printf("error!");

for(int i=0;i<img->NumLayers();i++) // iterate over all layers (if desired)
{
	// dimensions of this layer (outer boundaries of image content)
	int width;
	int height;
	// upper-left coordinate of bounding box, use this to place the layer accurately within the image
	int offset_x;
	int offset_y;
	DWORD* imagedata; // pixel array in ARGB format, need not be allocated (will point to stored image data)

	imagedata=img->GetLayerData(i, width, height, offset_x, offset_y); // get pointer to layer image, and info
	
	if(imagedata==NULL)
		printf("failed to get layer %i", i);
	
	if(img->LayerHidden(i))
		printf("this layer is hidden, but can be used anyway");
}

delete img; // note that this will free all the image data, so any pointers you might have to layers will be invalid

// (copy the layer image data if you want to keep it after the PSD object has been destroyed)

// it's safe to load a new PSD image using the same object, but all previous layer data will of course be lost

*/

class PSDImage
{
public:
	struct psd_layer
	{
		DWORD top, left, bottom, right;
		WORD channels;
		BYTE blendkey[4];
		BYTE opacity;
		BYTE flags;
		bool has_opacity;
		bool hidden;
		char name[32];

		DWORD* imagedata; // ARGB
	};

	struct psd_header
	{
		BYTE Signature[4];   /* File ID "8BPS" */
		WORD Version;        /* Version number, always 1 */
		BYTE Reserved[6];    /* Reserved, must be zeroed */
		WORD Channels;       /* Number of color channels (1-24) including alpha
								channels */
		DWORD Rows;           /* Height of image in pixels (1-30000) */
		DWORD Columns;        /* Width of image in pixels (1-30000) */
		WORD Depth;          /* Number of bits per channel (1, 8, and 16) */
		WORD Mode;           /* Color mode */
	};

	PSDImage()
	{
		layers=NULL;
		numlayers=0;
		loaded=false;
	}
	
	~PSDImage()
	{
		psd_free();
	}

	const psd_header& GetHeader() const { return header; }
	
	bool Load(const char* filename)
	{
		psd_free();

		FILE* file=fopen(filename, "rb");
		if(!file)
			return false;

		if(!psd_loadheader(file)) return false;
		if(!psd_loadcolormode(file)) return false;
		if(!psd_loadimageresources(file)) return false;
		if(!psd_loadlayerinfo(file))
		{
			psd_free();
			return false;
		}
		if(!psd_loadchannels(file))
		{
			psd_free();
			return false;
		}
		fclose(file);

		loaded=true;
		return true;
	}
	
	int NumLayers()
	{
		return numlayers;
	}

	int NumHiddenLayers()
	{
		for (int i = 0; i < numlayers; i++)
		{

		}
	}
	
	bool LayerHidden(int i)
	{
		if(i<0 || i>=numlayers) return true;
		return layers[i].hidden;
	}

	DWORD* GetLayerData(int i, int& width, int& height, int& offset_x, int& offset_y)
	{
		if(i<0 || i>=numlayers) return NULL;
		width=layers[i].right-layers[i].left;
		height=layers[i].bottom-layers[i].top;
		offset_x=layers[i].left;
		offset_y=layers[i].top;
		return layers[i].imagedata;
	}

    const psd_layer * GetLayer(int i)
        {
            if (i < 0 || i >= numlayers) return NULL;
            return &layers[i];
        }
	
private:
	psd_header header;
	psd_layer* layers;
	int numlayers;
	bool loaded;

	void psd_free()
	{
		for(int i=0;i<numlayers;i++)
			delete[] layers[i].imagedata;
		delete[] layers;
		layers=NULL;
		numlayers=0;
		loaded=false;
	}

	WORD endian_word(WORD x)
	{
		return (x>>8)|(x<<8);
	}
	
	DWORD endian_dword(DWORD x)
	{
		return (x>>24)|((x>>8)&0xFF00)|((x<<8)&0xFF0000)|((x<<24)&0xFF000000);
	}
	
	void endianify_word(WORD& x)
	{
		x=endian_word(x);
	}
	
	void endianify_dword(DWORD& x)
	{
		x=endian_dword(x);
	}
	
	bool psd_loadheader(FILE* file);
	
	bool psd_loadcolormode(FILE* file);
	
	bool psd_loadimageresources(FILE* file);
	
	bool psd_loadlayerinfo(FILE* file);
	
	bool psd_loadchannels(FILE* file);
};

#endif

