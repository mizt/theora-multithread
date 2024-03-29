/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2007                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id$

 ********************************************************************/

#include <stdlib.h>
#include "codec_internal.h"

#define ARM_NEON
#ifdef ARM_NEON
    #import <arm_neon.h>
#endif

#define DSP_OP_AVG(a,b) ((((int)(a)) + ((int)(b)))/2)
#define DSP_OP_DIFF(a,b) (((int)(a)) - ((int)(b)))
#define DSP_OP_ABS_DIFF(a,b) abs((((int)(a)) - ((int)(b))))

static void sub8x8__c(unsigned char *FiltPtr, unsigned char *ReconPtr,ogg_int16_t *DctInputPtr, ogg_uint32_t PixelsPerLine,ogg_uint32_t ReconPixelsPerLine) {
    
    int len = 8;
    while(len--) {
        
#ifndef ARM_NEON
    
        DctInputPtr[0] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[0],ReconPtr[0]);
        DctInputPtr[1] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[1],ReconPtr[1]);
        DctInputPtr[2] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[2],ReconPtr[2]);
        DctInputPtr[3] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[3],ReconPtr[3]);
        DctInputPtr[4] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[4],ReconPtr[4]);
        DctInputPtr[5] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[5],ReconPtr[5]);
        DctInputPtr[6] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[6],ReconPtr[6]);
        DctInputPtr[7] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[7],ReconPtr[7]);
        
#else
    
        vst1q_s16(DctInputPtr,vsubl_u8(vld1_u8(FiltPtr),vld1_u8(ReconPtr)));
       
#endif
        
        FiltPtr+=PixelsPerLine;
        ReconPtr+=ReconPixelsPerLine;
        DctInputPtr+=8;
    }
}

static void sub8x8_128__c(unsigned char *FiltPtr, ogg_int16_t *DctInputPtr,ogg_uint32_t PixelsPerLine) {
    
    int len = 8;
    while(len--) {
        
#ifndef ARM_NEON
    
        // INTRA mode so code raw image data
        // We convert the data to 8 bit signed (by subtracting 128) as this reduces the internal precision requirments in the DCT transform.
        DctInputPtr[0] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[0],128);
        DctInputPtr[1] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[1],128);
        DctInputPtr[2] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[2],128);
        DctInputPtr[3] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[3],128);
        DctInputPtr[4] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[4],128);
        DctInputPtr[5] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[5],128);
        DctInputPtr[6] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[6],128);
        DctInputPtr[7] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[7],128);
        
#else

        vst1q_s16(DctInputPtr,vsubl_u8(vld1_u8(FiltPtr),vdup_n_u8(128)));

#endif
    
        FiltPtr+=PixelsPerLine;
        DctInputPtr+=8;
    }
        
}

static void sub8x8avg2__c(unsigned char *FiltPtr, unsigned char *ReconPtr1, unsigned char *ReconPtr2, ogg_int16_t *DctInputPtr, ogg_uint32_t PixelsPerLine, ogg_uint32_t ReconPixelsPerLine) {
    
    int len=8;
    while(len--) {
        
#ifndef ARM_NEON
    
        DctInputPtr[0] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[0],DSP_OP_AVG(ReconPtr1[0],ReconPtr2[0]));
        DctInputPtr[1] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[1],DSP_OP_AVG(ReconPtr1[1],ReconPtr2[1]));
        DctInputPtr[2] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[2],DSP_OP_AVG(ReconPtr1[2],ReconPtr2[2]));
        DctInputPtr[3] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[3],DSP_OP_AVG(ReconPtr1[3],ReconPtr2[3]));
        DctInputPtr[4] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[4],DSP_OP_AVG(ReconPtr1[4],ReconPtr2[4]));
        DctInputPtr[5] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[5],DSP_OP_AVG(ReconPtr1[5],ReconPtr2[5]));
        DctInputPtr[6] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[6],DSP_OP_AVG(ReconPtr1[6],ReconPtr2[6]));
        DctInputPtr[7] = (ogg_int16_t)DSP_OP_DIFF(FiltPtr[7],DSP_OP_AVG(ReconPtr1[7],ReconPtr2[7]));
      
#else
        
        vst1q_s16(DctInputPtr,vsubl_u8(vld1_u8(FiltPtr),vsra_n_u8(vld1_u8(ReconPtr1),vld1_u8(ReconPtr2),1)));
           
#endif
    
        FiltPtr+=PixelsPerLine;
        ReconPtr1+=ReconPixelsPerLine;
        ReconPtr2+=ReconPixelsPerLine;
        DctInputPtr+=8;
    }
}

static ogg_uint32_t row_sad8__c(unsigned char *Src1, unsigned char *Src2) {
    
    unsigned int SadValue;
    unsigned int SadValue1;
    
#ifndef ARM_NEON
    
    SadValue = DSP_OP_ABS_DIFF(Src1[0],Src2[0])+
        DSP_OP_ABS_DIFF(Src1[1],Src2[1])+
        DSP_OP_ABS_DIFF(Src1[2],Src2[2])+
        DSP_OP_ABS_DIFF(Src1[3],Src2[3]);
    
    SadValue1 = DSP_OP_ABS_DIFF(Src1[4],Src2[4])+
        DSP_OP_ABS_DIFF(Src1[5],Src2[5])+
        DSP_OP_ABS_DIFF(Src1[6],Src2[6])+
        DSP_OP_ABS_DIFF(Src1[7],Src2[7]);
    
#else
    
    uint8x8_t res = vabd_u8(vld1_u8(Src1),vld1_u8(Src2));
    SadValue  = res[0]+res[1]+res[2]+res[3];
    SadValue1 = res[4]+res[5]+res[6]+res[7];
    
#endif
    
    return (SadValue>SadValue1)?SadValue:SadValue1;
    
}

static ogg_uint32_t col_sad8x8__c(unsigned char *Src1, unsigned char *Src2, ogg_uint32_t stride) {
    
    unsigned int MaxSad = 0;
    
#ifndef ARM_NEON
    
    ogg_uint32_t SadValue[8] = {0,0,0,0,0,0,0,0};
    ogg_uint32_t SadValue2[8] = {0,0,0,0,0,0,0,0};

    for(int i=0; i<4; i++) {
        SadValue[0]+=abs(Src1[0]-Src2[0]);
        SadValue[1]+=abs(Src1[1]-Src2[1]);
        SadValue[2]+=abs(Src1[2]-Src2[2]);
        SadValue[3]+=abs(Src1[3]-Src2[3]);
        SadValue[4]+=abs(Src1[4]-Src2[4]);
        SadValue[5]+=abs(Src1[5]-Src2[5]);
        SadValue[6]+=abs(Src1[6]-Src2[6]);
        SadValue[7]+=abs(Src1[7]-Src2[7]);
        
        Src1+=stride;
        Src2+=stride;
    }
    
    for(int i=0; i<4; i++) {
        SadValue2[0]+=abs(Src1[0]-Src2[0]);
        SadValue2[1]+=abs(Src1[1]-Src2[1]);
        SadValue2[2]+=abs(Src1[2]-Src2[2]);
        SadValue2[3]+=abs(Src1[3]-Src2[3]);
        SadValue2[4]+=abs(Src1[4]-Src2[4]);
        SadValue2[5]+=abs(Src1[5]-Src2[5]);
        SadValue2[6]+=abs(Src1[6]-Src2[6]);
        SadValue2[7]+=abs(Src1[7]-Src2[7]);
        
        Src1+=stride;
        Src2+=stride;
    }
    
#else
    
    uint16x8_t SadValue = vdupq_n_p16(0);
    uint16x8_t SadValue2 = vdupq_n_p16(0);
    
    for(int i=0; i<4; i++) {
        SadValue = vaddw_u8(SadValue,vabd_u8(vld1_u8(Src1),vld1_u8(Src2)));
        Src1+=stride;
        Src2+=stride;
    }
    
    for(int i=0; i<4; i++) {
        SadValue2 = vaddw_u8(SadValue2,vabd_u8(vld1_u8(Src1),vld1_u8(Src2)));
        Src1+=stride;
        Src2+=stride;
    }
    
#endif
    
    for(int i=0; i<8; i++) {
        if(SadValue[i]>MaxSad) MaxSad = SadValue[i];
    }
    
    for(int i=0; i<8; i++) {
        if(SadValue2[i]>MaxSad) MaxSad = SadValue2[i];
    }
    
    return MaxSad;
    
}

static ogg_uint32_t sad8x8__c(unsigned char *ptr1, ogg_uint32_t stride1, unsigned char *ptr2, ogg_uint32_t stride2) {
    
    unsigned int sad = 0;
    int len = 8;
    while(len--) {
        
#ifndef ARM_NEON
        
        sad+=DSP_OP_ABS_DIFF(ptr1[0],ptr2[0]);
        sad+=DSP_OP_ABS_DIFF(ptr1[1],ptr2[1]);
        sad+=DSP_OP_ABS_DIFF(ptr1[2],ptr2[2]);
        sad+=DSP_OP_ABS_DIFF(ptr1[3],ptr2[3]);
        sad+=DSP_OP_ABS_DIFF(ptr1[4],ptr2[4]);
        sad+=DSP_OP_ABS_DIFF(ptr1[5],ptr2[5]);
        sad+=DSP_OP_ABS_DIFF(ptr1[6],ptr2[6]);
        sad+=DSP_OP_ABS_DIFF(ptr1[7],ptr2[7]);
    
#else
       
        sad+=vaddlv_u8(vabd_u8(vld1_u8(ptr1),vld1_u8(ptr2)));
    
#endif
    
        ptr1+=stride1;
        ptr2+=stride2;
    }
    
    return sad;
    
}

static ogg_uint32_t sad8x8_thres__c(unsigned char *ptr1, ogg_uint32_t stride1, unsigned char *ptr2, ogg_uint32_t stride2, ogg_uint32_t thres) {
    
    unsigned int sad = 0;
    int len = 8;
    while(len--) {
    
#ifndef ARM_NEON
    
        sad+=DSP_OP_ABS_DIFF(ptr1[0],ptr2[0]);
        sad+=DSP_OP_ABS_DIFF(ptr1[1],ptr2[1]);
        sad+=DSP_OP_ABS_DIFF(ptr1[2],ptr2[2]);
        sad+=DSP_OP_ABS_DIFF(ptr1[3],ptr2[3]);
        sad+=DSP_OP_ABS_DIFF(ptr1[4],ptr2[4]);
        sad+=DSP_OP_ABS_DIFF(ptr1[5],ptr2[5]);
        sad+=DSP_OP_ABS_DIFF(ptr1[6],ptr2[6]);
        sad+=DSP_OP_ABS_DIFF(ptr1[7],ptr2[7]);
    
#else

        sad+=vaddlv_u8(vabd_u8(vld1_u8(ptr1),vld1_u8(ptr2)));
    
#endif
        
        if(sad>thres) break;
        
        ptr1+=stride1;
        ptr2+=stride2;
    }
    
    return sad;
    
}

static ogg_uint32_t sad8x8_xy2_thres__c(unsigned char *SrcData, ogg_uint32_t SrcStride, unsigned char *RefDataPtr1, unsigned char *RefDataPtr2, ogg_uint32_t RefStride, ogg_uint32_t thres) {
    
    unsigned int  sad = 0;
    
    unsigned int  len=8;
    while(len--) {

#ifndef ARM_NEON
    
        sad+=DSP_OP_ABS_DIFF(SrcData[0],DSP_OP_AVG(RefDataPtr1[0],RefDataPtr2[0]));
        sad+=DSP_OP_ABS_DIFF(SrcData[1],DSP_OP_AVG(RefDataPtr1[1],RefDataPtr2[1]));
        sad+=DSP_OP_ABS_DIFF(SrcData[2],DSP_OP_AVG(RefDataPtr1[2],RefDataPtr2[2]));
        sad+=DSP_OP_ABS_DIFF(SrcData[3],DSP_OP_AVG(RefDataPtr1[3],RefDataPtr2[3]));
        sad+=DSP_OP_ABS_DIFF(SrcData[4],DSP_OP_AVG(RefDataPtr1[4],RefDataPtr2[4]));
        sad+=DSP_OP_ABS_DIFF(SrcData[5],DSP_OP_AVG(RefDataPtr1[5],RefDataPtr2[5]));
        sad+=DSP_OP_ABS_DIFF(SrcData[6],DSP_OP_AVG(RefDataPtr1[6],RefDataPtr2[6]));
        sad+=DSP_OP_ABS_DIFF(SrcData[7],DSP_OP_AVG(RefDataPtr1[7],RefDataPtr2[7]));
        
#else
        
        sad+=vaddlv_u8(vabd_u8(vld1_u8(SrcData),vsra_n_u8(vld1_u8(RefDataPtr1),vld1_u8(RefDataPtr2),1)));
            
#endif
    
        if(sad>thres) break;
        
        SrcData+=SrcStride;
        RefDataPtr1+=RefStride;
        RefDataPtr2+=RefStride;
    }

    return sad;
}

static ogg_uint32_t intra8x8_err__c(unsigned char *DataPtr, ogg_uint32_t Stride) {
    
    unsigned int  XSum=0;
    unsigned int  XXSum=0;
    
    unsigned int  len = 8;
    while(len--) {
        
#ifndef ARM_NEON
    
        // Examine alternate pixel locations.
        XSum+=DataPtr[0];
        XXSum+=DataPtr[0]*DataPtr[0];
        XSum+=DataPtr[1];
        XXSum+=DataPtr[1]*DataPtr[1];
        XSum+=DataPtr[2];
        XXSum+=DataPtr[2]*DataPtr[2];
        XSum+=DataPtr[3];
        XXSum+=DataPtr[3]*DataPtr[3];
        XSum+=DataPtr[4];
        XXSum+=DataPtr[4]*DataPtr[4];
        XSum+=DataPtr[5];
        XXSum+=DataPtr[5]*DataPtr[5];
        XSum+=DataPtr[6];
        XXSum+=DataPtr[6]*DataPtr[6];
        XSum+=DataPtr[7];
        XXSum+=DataPtr[7]*DataPtr[7];
        
        // Step to next row of block.
        DataPtr+=Stride;
        
#else
    
        uint16x8_t x = vmovl_u8(vld1_u8(DataPtr));
        XSum+=x[0];
        XSum+=x[1];
        XSum+=x[2];
        XSum+=x[3];
        XSum+=x[4];
        XSum+=x[5];
        XSum+=x[6];
        XSum+=x[7];
        
        uint16x8_t xx = vmulq_u16(x,x);
        XXSum+=xx[0];
        XXSum+=xx[1];
        XXSum+=xx[2];
        XXSum+=xx[3];
        XXSum+=xx[4];
        XXSum+=xx[5];
        XXSum+=xx[6];
        XXSum+=xx[7];
        
        DataPtr+=(Stride-8);
    
#endif
        
    }

    // Compute population variance as mis-match metric.
    return (((XXSum<<6)-XSum*XSum));
    
}

static ogg_uint32_t inter8x8_err__c(unsigned char *SrcData, ogg_uint32_t SrcStride, unsigned char *RefDataPtr, ogg_uint32_t RefStride) {
    
    int XSum=0;
    int XXSum=0;
    
    int len = 8;
    while(len--) {
        
#ifndef ARM_NEON
    
        int DiffVal = DSP_OP_DIFF(SrcData[0],RefDataPtr[0]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[1],RefDataPtr[1]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[2],RefDataPtr[2]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[3],RefDataPtr[3]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[4],RefDataPtr[4]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[5],RefDataPtr[5]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[6],RefDataPtr[6]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[7],RefDataPtr[7]);
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
       
#else
      
        int16x8_t x = (int16x8_t)(vsubl_u8(vld1_u8(SrcData),vld1_u8(RefDataPtr)));
        
        XSum+=x[0];
        XSum+=x[1];
        XSum+=x[2];
        XSum+=x[3];
        XSum+=x[4];
        XSum+=x[5];
        XSum+=x[6];
        XSum+=x[7];
        
        uint16x8_t xx = vmulq_u16(x,x);
        
        XXSum+=xx[0];
        XXSum+=xx[1];
        XXSum+=xx[2];
        XXSum+=xx[3];
        XXSum+=xx[4];
        XXSum+=xx[5];
        XXSum+=xx[6];
        XXSum+=xx[7];
    
#endif
        
        SrcData+=SrcStride;
        RefDataPtr+=RefStride;
    }
    
    // Compute and return population variance as mis-match metric.
    return (((XXSum<<6)-XSum*XSum));
}

static ogg_uint32_t inter8x8_err_xy2__c(unsigned char *SrcData, ogg_uint32_t SrcStride, unsigned char *RefDataPtr1, unsigned char *RefDataPtr2, ogg_uint32_t RefStride) {
    
    int XSum=0;
    int XXSum=0;
    
    int len = 8;
    while(len--) {
        
#ifndef ARM_NEON
    
        int DiffVal = DSP_OP_DIFF(SrcData[0],DSP_OP_AVG(RefDataPtr1[0],RefDataPtr2[0]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[1],DSP_OP_AVG(RefDataPtr1[1],RefDataPtr2[1]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[2],DSP_OP_AVG(RefDataPtr1[2],RefDataPtr2[2]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[3],DSP_OP_AVG(RefDataPtr1[3],RefDataPtr2[3]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[4],DSP_OP_AVG(RefDataPtr1[4],RefDataPtr2[4]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[5],DSP_OP_AVG(RefDataPtr1[5],RefDataPtr2[5]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[6],DSP_OP_AVG(RefDataPtr1[6],RefDataPtr2[6]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
        DiffVal = DSP_OP_DIFF(SrcData[7],DSP_OP_AVG(RefDataPtr1[7],RefDataPtr2[7]));
        XSum+=DiffVal;
        XXSum+=DiffVal*DiffVal;
        
#else
        
        
        
    
        int16x8_t x = (int16x8_t)(vsubl_u8(vld1_u8(SrcData),vsra_n_u8(vld1_u8(RefDataPtr1),vld1_u8(RefDataPtr2),1)));
      
        XSum+=x[0];
        XSum+=x[1];
        XSum+=x[2];
        XSum+=x[3];
        XSum+=x[4];
        XSum+=x[5];
        XSum+=x[6];
        XSum+=x[7];
        
        uint16x8_t xx = vmulq_u16(x,x);
        
        XXSum+=xx[0];
        XXSum+=xx[1];
        XXSum+=xx[2];
        XXSum+=xx[3];
        XXSum+=xx[4];
        XXSum+=xx[5];
        XXSum+=xx[6];
        XXSum+=xx[7];
            
#endif
        
        SrcData+=SrcStride;
        RefDataPtr1+=RefStride;
        RefDataPtr2+=RefStride;
    }
    
    // Compute and return population variance as mis-match metric.
    return (((XXSum<<6)-XSum*XSum));
    
}

static void nop (void) { /* NOP */ }

void dsp_init(DspFunctions *funcs)
{
  /* TH_DEBUG("setting dsp functions to C defaults.\n"); */
  funcs->save_fpu = nop;
  funcs->restore_fpu = nop;
  funcs->sub8x8 = sub8x8__c;
  funcs->sub8x8_128 = sub8x8_128__c;
  funcs->sub8x8avg2 = sub8x8avg2__c;
  funcs->row_sad8 = row_sad8__c;
  funcs->col_sad8x8 = col_sad8x8__c;
  funcs->sad8x8 = sad8x8__c;
  funcs->sad8x8_thres = sad8x8_thres__c;
  funcs->sad8x8_xy2_thres = sad8x8_xy2_thres__c;
  funcs->intra8x8_err = intra8x8_err__c;
  funcs->inter8x8_err = inter8x8_err__c;
  funcs->inter8x8_err_xy2 = inter8x8_err_xy2__c;
}

void dsp_static_init(DspFunctions *funcs)
{
  ogg_uint32_t cpuflags;

  cpuflags = oc_cpu_flags_get ();
  dsp_init (funcs);

  dsp_recon_init (funcs, cpuflags);
  dsp_dct_init (funcs, cpuflags);
#if defined(USE_ASM)
  if (cpuflags & OC_CPU_X86_MMX) {
    dsp_mmx_init(funcs);
  }
# ifndef WIN32
  /* This is implemented for win32 yet */
  if (cpuflags & OC_CPU_X86_MMXEXT) {
    dsp_mmxext_init(funcs);
  }
# endif
#endif
}

