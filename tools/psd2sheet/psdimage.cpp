#include "psdimage.h"

#include <string.h>

bool PSDImage::psd_loadheader(FILE* file)
{
	if(file==NULL)
		return false;
	fread(header.Signature, 4, sizeof(BYTE), file);
	fread(&header.Version, 1, sizeof(WORD), file);
	fread(header.Reserved, 6, sizeof(BYTE), file);
	fread(&header.Channels, 1, sizeof(WORD), file);
	fread(&header.Rows, 1, sizeof(DWORD), file);
	fread(&header.Columns, 1, sizeof(DWORD), file);
	fread(&header.Depth, 1, sizeof(WORD), file);
	fread(&header.Mode, 1, sizeof(WORD), file);
	endianify_word(header.Version);
	endianify_word(header.Channels);
	endianify_dword(header.Rows);
	endianify_dword(header.Columns);
	endianify_word(header.Depth);
	endianify_word(header.Mode);
	char* sig=(char*)header.Signature;
	if(!(sig[0]=='8' && sig[1]=='B' && sig[2]=='P' && sig[3]=='S'))
		return false;
	return true;
}

bool PSDImage::psd_loadcolormode(FILE* file)
{
	DWORD size=0;
	fread(&size, 1, sizeof(DWORD), file);
	endianify_dword(size);
	BYTE junk;
	for(int i=0;i<(int)size;i++)
		fread(&junk, 1, 1, file);
	return true;
}

bool PSDImage::psd_loadimageresources(FILE* file)
{
	DWORD size=0;
	fread(&size, 1, sizeof(DWORD), file);
	endianify_dword(size);
	BYTE junk;
	for(int i=0;i<(int)size;i++)
		fread(&junk, 1, 1, file);
	return true;
}

bool PSDImage::psd_loadlayerinfo(FILE* file)
{
	DWORD totalsize=0;
	fread(&totalsize, 1, sizeof(DWORD), file);
	endianify_dword(totalsize);
	if(totalsize==0)
	{
		numlayers=1;
		layers[0].top=0;
		layers[0].left=0;
		layers[0].bottom=header.Rows;
		layers[0].right=header.Columns;
		layers[0].channels=header.Channels;
		return true;
	}
	DWORD layersize=0;
	fread(&layersize, 1, sizeof(DWORD), file);
	endianify_dword(layersize);
	WORD numrecords=0;
	fread(&numrecords, 1, sizeof(WORD), file);
	endianify_word(numrecords);
	if((short)numrecords<0)
		numrecords=(WORD)(-(short)numrecords); // "if negative, numlayers is the absolute value and first alpha channel is alpha for merged result" (flattened image?)
	numlayers=numrecords;
	
	layers=new psd_layer[numlayers];

	for(int i=0;i<numrecords;i++)
	{
		psd_layer& l=layers[i];
		fread(&l.top, 1, sizeof(DWORD), file);
		fread(&l.left, 1, sizeof(DWORD), file);
		fread(&l.bottom, 1, sizeof(DWORD), file);
		fread(&l.right, 1, sizeof(DWORD), file);
		fread(&l.channels, 1, sizeof(WORD), file);
		endianify_dword(l.top);
		endianify_dword(l.left);
		endianify_dword(l.bottom);
		endianify_dword(l.right);
		endianify_word(l.channels);
		DWORD chansize[24];
		for(int j=0;j<l.channels;j++)
		{
			WORD id=0;
			fread(&id, 1, sizeof(WORD), file);
			fread(&chansize[j], 1, sizeof(DWORD), file);
			endianify_dword(chansize[j]);
		}
		char sig[5];
		sig[4]='\0';
		fread(sig, 4, sizeof(BYTE), file);
		fread(sig, 4, sizeof(BYTE), file);
		memcpy(l.blendkey, sig, 4);
		fread(&l.opacity, 1, sizeof(BYTE), file);
		DWORD junk;
		fread(&junk, 1, sizeof(BYTE), file); // clipping
		fread(&l.flags, 1, sizeof(BYTE), file);
		l.has_opacity=true;
		if(l.flags&1)
			l.has_opacity=false;
		l.hidden=((l.flags&2)!=0);
		fread(&junk, 1, sizeof(BYTE), file); // padding
		DWORD extrasize=0;
		fread(&extrasize, 1, sizeof(DWORD), file); // extra bytes, total size of remaining data from this position
		endianify_dword(extrasize);
		DWORD extrastart=ftell(file);
		// layer mask
		DWORD masksize=0;
		fread(&masksize, 1, sizeof(DWORD), file);
		endianify_dword(masksize);
		if(masksize!=0)
			for(int b=0;b<20;b++)
				fread(&junk, 1, 1, file);
		DWORD blendsize=0;
		fread(&blendsize, 1, sizeof(DWORD), file);
		endianify_dword(blendsize);
		for(int b=0;b<(int)blendsize;b++)
			fread(&junk, 1, 1, file);

		BYTE namelen=0;
		fread(&namelen, 1, sizeof(BYTE), file);
		int actual_bytes=namelen;
		if((actual_bytes&1)==0) actual_bytes+=1; // size of pascal string padded to even number of bytes (including length byte, so #characters is uneven)
		fread(&l.name, actual_bytes, sizeof(char), file);
		l.name[namelen]='\0';

		fseek(file, extrastart+extrasize, SEEK_SET); // move to end of this layer block
	}
	return true;
}

bool PSDImage::psd_loadchannels(FILE* file)
{
	DWORD pos=ftell(file);
	fseek(file, 0, SEEK_END);
	DWORD end=ftell(file);
	fseek(file, pos, SEEK_SET);

	WORD compression=33; // unknown
	int curlayer=0;
	int layerchannel=0;
	int numchannels=0;
	while(ftell(file)<(long)end && curlayer<numlayers)
	{
		psd_layer& layer=layers[curlayer];
		int width=layer.right-layer.left;
		int height=layer.bottom-layer.top;
		
		if(layerchannel==0) // first channel in this layer, allocate ARGB data memory
		{
			layer.imagedata=new DWORD[width*height];
			if(layer.channels<4) // this layer has no alpha, so prep imagedata with opaque alpha values
				for(int i=0;i<width*height;i++)
					layer.imagedata[i]=0xFF000000;
		}

		if(!(curlayer==0 && layerchannel>0 && compression==0))
		{	// TODO: investigate whether or not this is true, might be that all raw layers keep the channels together without per-channel compression info
			fread(&compression, 1, sizeof(WORD), file);
			endianify_word(compression);
		}

		int colorchannel=layerchannel;
		if(layer.channels==4) // first channel is opacity for floating layers
			colorchannel-=1;

		if(compression==0) // raw
		{
			int pixi=0;
			for(int y=0;y<height;y++)
				for(int x=0;x<width;x++)
				{
					int wx=x+layer.left;
					int wy=y+layer.top;
					BYTE pixel=0;
					fread(&pixel, 1, sizeof(BYTE), file);
					DWORD& imgpixel=layer.imagedata[pixi];
					if(colorchannel==-1 && pixel!=0) imgpixel=(imgpixel&0x00FFFFFF)|(pixel<<24); // alpha
					if(colorchannel==0) imgpixel=(imgpixel&0xFF00FFFF)|(pixel<<16); // r
					if(colorchannel==1) imgpixel=(imgpixel&0xFFFF00FF)|(pixel<<8); // g
					if(colorchannel==2) imgpixel=(imgpixel&0xFFFFFF00)|(pixel<<0); // b
					pixi++;
				}
		}
		if(compression==1) // PackBits
		{
			int pixi=0;
			WORD* linesizes=new WORD[height];
			for(int y=0;y<height;y++)
			{
				fread(&linesizes[y], 1, sizeof(WORD), file);
				endianify_word(linesizes[y]);
			}
			for(int y=0;y<height;y++)
			{
				int wx=layer.left;
				int wy=y+layer.top;
				int bytesread=0;
				int count=0;
				while(bytesread<linesizes[y])
				{
					BYTE control=0;
					fread(&control, 1, sizeof(BYTE), file);
					bytesread++;
					if(control&0x80) // RLE
					{
						count=-(char)control+1;
						BYTE pixel=0;
						fread(&pixel, 1, sizeof(BYTE), file);
						bytesread++;
						for(int i=0;i<count;i++)
						{
							DWORD& imgpixel=layer.imagedata[pixi];
							if(colorchannel==-1 && pixel!=0) imgpixel=(imgpixel&0x00FFFFFF)|(pixel<<24); // alpha
							if(colorchannel==0) imgpixel=(imgpixel&0xFF00FFFF)|(pixel<<16); // r
							if(colorchannel==1) imgpixel=(imgpixel&0xFFFF00FF)|(pixel<<8); // g
							if(colorchannel==2) imgpixel=(imgpixel&0xFFFFFF00)|(pixel<<0); // b
							pixi++;
							wx++;
						}
					}
					else
					{
						count=control+1;
						for(int i=0;i<count;i++)
						{
							BYTE pixel=0;
							fread(&pixel, 1, sizeof(BYTE), file);
							bytesread++;
							DWORD& imgpixel=layer.imagedata[pixi];
							if(colorchannel==-1 && pixel!=0) imgpixel=(imgpixel&0x00FFFFFF)|(pixel<<24); // alpha
							if(colorchannel==0) imgpixel=(imgpixel&0xFF00FFFF)|(pixel<<16); // r
							if(colorchannel==1) imgpixel=(imgpixel&0xFFFF00FF)|(pixel<<8); // g
							if(colorchannel==2) imgpixel=(imgpixel&0xFFFFFF00)|(pixel<<0); // b
							pixi++;
							wx++;
						}
					}
				}
			}
			delete[] linesizes;
		}

		// TODO: could there be a pad byte here to get a word-aligned channel size? or is that per-layer? or per-row in the RLE compression? spec is vague, hooray

		numchannels++;
		layerchannel++;
		if(layerchannel>=layer.channels)
		{
			layerchannel=0;
			curlayer++;
		}
	}
	return true;
}

