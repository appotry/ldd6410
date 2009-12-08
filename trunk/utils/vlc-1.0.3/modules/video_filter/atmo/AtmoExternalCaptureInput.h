#ifndef _AtmoExternalCaptureInput_h_
#define _AtmoExternalCaptureInput_h_

#include "AtmoDefs.h"

#if defined(WIN32)
#   include <windows.h>
# else
#   if defined(_ATMO_VLC_PLUGIN_)
       // need bitmap info header
#      include <vlc_codecs.h>
#   endif
#endif

#if !defined(_ATMO_VLC_PLUGIN_)
#  include <comdef.h>		
#  include "AtmoWin_h.h"
#endif

#include "AtmoInput.h"
#include "AtmoThread.h"
#include "AtmoConfig.h"
#include "AtmoDynData.h"
#include "AtmoCalculations.h"


class CAtmoExternalCaptureInput :
      public CAtmoInput,
      public CThread
{
protected:
#if defined(_ATMO_VLC_PLUGIN_)
    vlc_cond_t   m_WakeupCond;
    vlc_mutex_t  m_WakeupLock;
#else
    HANDLE m_hWakeupEvent;
#endif

    BITMAPINFOHEADER m_CurrentFrameHeader;
    void *m_pCurrentFramePixels;

    virtual DWORD Execute(void);
    void CalcColors();

public:
    /*
       this method is called from the com server AtmoLiveViewControlImpl!
       or inside videolan from the filter method to start a new processing
    */
    void DeliverNewSourceDataPaket(BITMAPINFOHEADER *bmpInfoHeader,void *pixelData);

public:
    CAtmoExternalCaptureInput(CAtmoDynData *pAtmoDynData);
    virtual ~CAtmoExternalCaptureInput(void);

    /*
       Opens the input-device. Parameters (e.g. the device-name)
       Returns true if the input-device was opened successfully.
       input-device can be the GDI surface of screen (windows only)
       or the videolan filter
    */
    virtual ATMO_BOOL Open(void);

    /*
     Closes the input-device.
     Returns true if the input-device was closed successfully.
    */
    virtual ATMO_BOOL Close(void);

    /*
      this method is called from the AtmoLiveView thread - to get the
      new color packet (a packet is an RGB triple for each channel)
    */
    virtual tColorPacket GetColorPacket(void);

    /*
      this method is also called from the AtmoLiveView thread - to
      resync on a frame
    */
    virtual void WaitForNextFrame(DWORD timeout);
};

#endif
