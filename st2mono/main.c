//
//  main.c
//  st2mono
//
//  Created by Xavier Lizarraga on 09/05/17.
//  Copyright © 2017 Xavier Lizarraga. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#define NFRAMES 100

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_CHANNEL, ARG_NARGS};


int main(int argc, const char * argv[]) {
    int framesread;
    PSF_PROPS inprops, outprops;
    psf_format outformat = PSF_STDWAVE;
    
    /* init all resource vals to default states */
    int ifd=-1, ofd=-1;
    int error=0, chan = -1;
    float* inbuffer = NULL;
    float* outbuffer = NULL;
    
    printf("ST2MONO: convert stereo soundfile to mono\n");
    
    if (argc!=ARG_NARGS)
    {
        printf("ERROR: insufficient number of arguments.\n"
               "USAGE: mono2stereo infile outfile channel\n"
               "channel: defines channel to copy from infile (0-1)\n");
        return 1;
    }
    
    chan = atoi(argv[ARG_CHANNEL]);
    if (chan<0 || chan >1){
        printf("Error: channel input must be defined as 0 or 1");
        error++;
        goto exit;
    }
    
    printf("Copying channel %i from infile %s\n", chan, argv[ARG_INFILE]);
    
    /* start up portsf */
    if (psf_init())
    {
        printf("ERROR: unable to start portsf.\n");
        return 1;
    }
    
    /* open infile */
    ifd = psf_sndOpen(argv[ARG_INFILE],&inprops,0);
    if (ifd<0)
    {
        printf("ERROR: unable to open \"%s\"\n",argv[ARG_INFILE]);
        return 1;
    }
    
    /* we now have a resource, so we use goto here after on hitting any error */
    /* tell user if source file is already floats */
    if (inprops.chans == 1){
        printf ( " Info: infile is already a mono audio file. \r" ) ;
    }
    
    printf("infile has %i channels\n", inprops.chans);
    printf("infile has a sample type %u\n", inprops.samptype);
    printf("infile has defined a format %u\n", inprops.format);
    /* properties of infile and outfile will be the same except
     infile is mono and outfile is stereo */
    outprops = inprops;
    outprops.chans = 1;
    
    ofd = psf_sndCreate(argv[2], &outprops,0,0,PSF_CREATE_RDWR);
    if(ofd < 0){
        printf("Error: unable to create outfile %s in mono channel format\n", argv[ARG_INFILE]);
        error++;
        goto exit;
    }
    
    /* check if outfile extension is one we know about */
    outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
    if (outformat==PSF_FMT_UNKNOWN)
    {
        printf("Outfile name \"%s\" has unknown format.\n"
               "Use any of .wav .aiff .aif .afc .aifc\n",
               argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }
    
    /* allocate space for input frame buffer */
    inbuffer = (float*)malloc(sizeof(float)*inprops.chans*NFRAMES);
    /* allocate space for output frame buffer */
    outbuffer = (float*)malloc(sizeof(float)*outprops.chans*NFRAMES);
    
    while ((framesread = psf_sndReadFloatFrames(ifd,inbuffer,NFRAMES)) > 0)
    {
        //printf("%u\n",framesread);
        int i, out_i;
        for (i=0, out_i = 0; i < framesread; i++){
            if (chan==0){  // left channel
                outbuffer[i] = (float)(inbuffer[out_i++]);
                out_i++;
            }
            else{                   // right channel
                out_i++;
                outbuffer[i] = (float)(inbuffer[out_i++]);
            }
        }
        if (psf_sndWriteFloatFrames(ofd,outbuffer,framesread) != framesread)
        {
            printf("Error writing to outfile.\n");
            error++;
            break;
        }
    }
    printf("Done.\n");
    
    /* TODO - Detect automatically which is the channel with higher peak. Analyse left and right channel and copy the channel with maximum RMS. */
    /* TODO - copy infile in a char to generate outfile name as infile+"_mono" */
    
    /* clean up resources */
exit:
    if (ifd)
        psf_sndClose(ifd);
    if (ofd)
        psf_sndClose(ofd);
    if (inbuffer)
        free(inbuffer);
    if (outbuffer)
        free(outbuffer);
    psf_finish();
    
    return error;
}
