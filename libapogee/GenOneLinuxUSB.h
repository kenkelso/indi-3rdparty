/*! 
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright(c) 2010 Apogee Instruments, Inc. 
* \class GenOneLinuxUSB 
* \brief Usb interface for *nix systems.
* 
*/ 


#ifndef GENONELINUXUSB_INCLUDE_H__ 
#define GENONELINUXUSB_INCLUDE_H__ 

#include <string>
#include <vector>
#include <libusb-1.0/libusb.h>

#include "../IUsb.h"

class GenOneLinuxUSB : public IUsb
{ 
    public: 
		GenOneLinuxUSB( const uint16_t DeviceNum );
		virtual ~GenOneLinuxUSB();

		uint16_t ReadReg(uint16_t FpgaReg);

		void WriteReg(uint16_t FpgaReg,
			const uint16_t FpgaData );

		void GetVendorInfo(uint16_t & VendorId,
			uint16_t & ProductId, uint16_t  & DeviceId);

		void SetupSingleImgXfer(uint16_t Rows,
			uint32_t Cols);

		void SetupSequenceImgXfer(uint16_t Rows,
			uint16_t Cols, uint16_t NumOfImages);

        void CancelImgXfer();

		void ReadImage(uint16_t * ImageData,
					   const uint32_t InSizeInBytes,
					   uint32_t &OutSizeInBytes);

		void GetStatus(uint8_t * status, uint32_t NumBytes);

		void UsbRequestIn(uint8_t RequestCode,
			uint16_t	Index, uint16_t	Value,
			uint8_t * ioBuf, uint32_t BufSzInBytes);

		void UsbRequestOut(uint8_t RequestCode,
			uint16_t	Index, uint16_t Value,
			const uint8_t * ioBuf, uint32_t BufSzInBytes);

		void GetSerialNumber(int8_t * ioBuf, uint32_t BufSzInBytes);

		void GetUsbFirmwareVersion(int8_t * ioBuf, uint32_t BufSzInBytes);

        std::string GetDriverVersion();

        bool IsError();

        uint16_t GetDeviceNum() { return m_DeviceNum; }

        void UsbReqOutWithExtendedTimeout(uint8_t RequestCode,
            uint16_t Index, uint16_t	Value,
            const uint8_t * ioBuf, uint32_t BufSzInBytes);

        void ReadSerialPort( uint16_t PortId, 
            uint8_t * ioBuf, uint16_t BufSzInBytes );

    private:
		 bool OpenDeviceHandle(const uint16_t DeviceNum, std::string & err);
		 libusb_context * m_Context;
		 libusb_device_handle  * m_Device;
		 libusb_device_descriptor m_DeviceDescriptor;
		 const std::string m_fileName;
         bool m_ReadImgError;
         bool m_IoError;
         int32_t m_NumRegWriteRetries;
         uint16_t m_DeviceNum;

        //disabling the copy ctor and assignment operator
        //generated by the compiler - don't want them
        //Effective C++ Item 6
        GenOneLinuxUSB(const GenOneLinuxUSB&);
        GenOneLinuxUSB& operator=(GenOneLinuxUSB&);
}; 

#endif