#include "ffmpeg.h"

#include <iostream>
using namespace std;

#if defined(WIN32)
#include <windows.h>
#include <dshow.h>
#elif defined(LINUX)

#endif

Hardware::~Hardware() { }

QList<QString> Hardware::getNames() const { return names; }
QList<QString> Hardware::getFormats() const { return formats; }
QList<QString> Hardware::getFiles() const { return files; }

//TODO: Implement
VideoHardware::VideoHardware() {
#if defined(LINUX)
   names.append("Default webcam");
   formats.append("v4l2");
   files.append("/dev/video1");
#elif defined(WIN32)
   /* Fucking Microsoft
   HRESULT hr;
   ICreateDevEnum *pSysDevEnum = NULL;
   hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER
                        , IID_ICreateDevEnum, (void**)&pSysDevEnum);
   if(FAILED(hr)) { qWarning("Can't enumerate devices"); return; }
   IEnumMoniker *pEnumCat = NULL;
   hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
   if(hr == S_OK) {
      IMoniker *pMoniker = NULL;
      ULONG cFetched;
      while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
         IPropertyBag *pProbBag;
         hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void**)&pPropBag);
         if(SUCEEDED(hr)) {
            VARIANT varName;
            VariantInit(&varName);
            varName.vt = VT_BSTR;
            hr = propBag->Read(L"FriendlyName", &varName, 0);
            if(SUCEEDED(hr)) {
               names.append(QString::fromUtf8(varName.bstrVal));
               formats.append("dshow");
               files.append("video="+QString::fromUtf8(varName.bstrVal));
            }
            VariantClear(&varName);
            pPropBag->Release();
         }
         pMoniker->Release();
      }
      pEnumCat->Release();
   } else {
      qWarning("Can't enumerate video devices");
   }
   pSysDevEnum->Release();
   */

   names.append("Default webcam");
   formats.append("dshow");
   files.append("video=Venus USB2.0 Camera");
#endif
}

//TODO: Implement
AudioHardware::AudioHardware() {
#if defined(LINUX)
   names.append("Default microphone");
   formats.append("alsa");
   files.append("hw:0");
#elif defined(WIN32)
   names.append("Default microphone");
   formats.append("dshow");
   files.append("audio=Venus USB2.0 Camera");
#endif
}
