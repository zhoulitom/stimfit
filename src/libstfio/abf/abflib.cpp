// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.



#include <string>
#include <iomanip>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <sstream>

#if defined(__LINUX__) || defined(__STF__) || defined(__APPLE__)
#include "./axon/Common/axodefn.h"
#include "./axon/AxAbfFio32/abffiles.h"
#include "./axon2/ProtocolReaderABF2.hpp"
#endif

#include "./abflib.h"
#include "../recording.h"

namespace stfio {

std::string ABF1Error(const std::string& fName, int nError);

std::string dateToStr(ABFLONG date);
std::string timeToStr(ABFLONG time);

}

std::string stfio::ABF1Error(const std::string& fName, int nError) {
    UINT uMaxLen=320;
    std::vector<char> errorMsg(uMaxLen);
    // local copy:
    std::string wxCp = fName;
    ABF_BuildErrorText(nError, wxCp.c_str(),&errorMsg[0], uMaxLen );
    return std::string( &errorMsg[0] );
}

std::string stfio::dateToStr(ABFLONG date) {
    std::ostringstream dateStream;
    ldiv_t year=ldiv(date,(ABFLONG)10000);
    dateStream << year.quot;
    ldiv_t month=ldiv(year.rem,(ABFLONG)100);
    dateStream << "/" << month.quot;
    dateStream << "/" << month.rem;
    return dateStream.str();
}

std::string stfio::timeToStr(ABFLONG time) {
    std::ostringstream timeStream;
    ldiv_t hours=ldiv(time,(ABFLONG)3600);
    timeStream << hours.quot;
    ldiv_t minutes=ldiv(hours.rem,(ABFLONG)60);
    if (minutes.quot<10)
        timeStream << ":" << '0' << minutes.quot;
    else
        timeStream << ":" << minutes.quot;
    if (minutes.rem<10)
        timeStream << ":" << '0' << minutes.rem;
    else
        timeStream << ":" << minutes.rem;
    return timeStream.str();
}

void stfio::importABFFile(const std::string &fName, Recording &ReturnData, ProgressInfo& progDlg) {
    ABF2_FileInfo fileInfo;

    // Open file:
#ifndef _WINDOWS
    FILE* fh = fopen( fName.c_str(), "r" );
    if (!fh) {
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        fclose(fh);
        throw std::runtime_error(errorMsg);
    }

    // attempt to read first chunk of data:
    int res = fseek( fh, 0, SEEK_SET);
    if (res != 0) {
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        fclose(fh);
        throw std::runtime_error(errorMsg);
    }
    res = fread( &fileInfo, sizeof( fileInfo ), 1, fh );
    if (res != 1) {
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        fclose(fh);
        throw std::runtime_error(errorMsg);
    }
    fclose(fh);
#else
    std::wstringstream fNameS;
    fNameS << fName.c_str();
    HANDLE hFile = CreateFile(fNameS.str().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if (hFile == INVALID_HANDLE_VALUE) { 
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        CloseHandle(hFile);
        throw std::runtime_error(errorMsg);
    }

	// Read one character less than the buffer size to save room for
    // the terminating NULL character.
    DWORD dwBytesRead = 0;

    if( FALSE == ReadFile(hFile, &fileInfo, sizeof( fileInfo ), &dwBytesRead, NULL) ) {
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        CloseHandle(hFile);
        throw std::runtime_error(errorMsg);
    }

	if (dwBytesRead <= 0) {
        std::string errorMsg("Exception while calling importABFFile():\nCouldn't open file");
        CloseHandle(hFile);
        throw std::runtime_error(errorMsg);
    }
    CloseHandle(hFile);
#endif
    
    if (CABF2ProtocolReader::CanOpen( (void*)&fileInfo, sizeof(fileInfo) )) {
        importABF2File( std::string(fName.c_str()), ReturnData, progDlg );
    } else {
        importABF1File( std::string(fName.c_str()), ReturnData, progDlg );
    }
}


void stfio::importABF2File(const std::string &fName, Recording &ReturnData, ProgressInfo& progDlg) {

    CABF2ProtocolReader abf2;
    std::wstring wfName;
    wfName.resize(fName.size());
    std::copy(fName.begin(), fName.end(), wfName.begin());
    // for(std::string::size_type i=0; i<fName.size(); ++i) {
    //     wfName[i] = (wchar_t)fName[i];
    // }

    if (!abf2.Open( &wfName[0] )) {
        std::string errorMsg("Exception while calling importABF2File():\nCouldn't open file");
        throw std::runtime_error(errorMsg);
        abf2.Close();
    }
#ifdef _STFDEBUG
    else {
        std::cout << "File successfully opened" << std::endl;
    }
#endif
    int nError = 0;
    if (!abf2.Read( &nError )) {
        std::string errorMsg("Exception while calling importABF2File():\nCouldn't read file");
        throw std::runtime_error(errorMsg);
        abf2.Close();
    }
            
    const ABF2FileHeader* pFH = abf2.GetFileHeader();
#ifdef _STFDEBUG
    std::cout << "ABF2 file information" << std::endl
              << "File version " <<  pFH->fFileVersionNumber << std::endl
              << "Header version " <<  pFH->fHeaderVersionNumber << std::endl
              << "Data format " << pFH->nDataFormat << std::endl
              << "Number of channels " << pFH->nADCNumChannels << std::endl
              << "Number of sweeps " << pFH->lActualEpisodes << std::endl
              << "Sampling points per sweep " << pFH->lNumSamplesPerEpisode << std::endl
              << "File type " << pFH->nOperationMode << std::endl;
#endif
    
    int numberChannels = pFH->nADCNumChannels;
    ABFLONG numberSections = pFH->lActualEpisodes;
    ABFLONG finalSections = numberSections;
    bool gapfree = (pFH->nOperationMode == ABF2_GAPFREEFILE);
    if (gapfree) {
        finalSections = 1;
    }
    int hFile = abf2.GetFileNumber();
    for (int nChannel=0; nChannel < numberChannels; ++nChannel) {
        int progbar = (int)(((double)nChannel/(double)numberChannels)*100.0);
        progDlg.Update(progbar, "Memory allocation");
        ABFLONG grandsize = pFH->lNumSamplesPerEpisode / numberChannels;
        std::ostringstream label;
        label  
               << fName
               << ", gapfree section";
        if (gapfree) {
            grandsize = pFH->lActualAcqLength / numberChannels;
            Vector_double test_size(0);
            ABFLONG maxsize = test_size.max_size()
#ifdef _WINDOWS
                // doesn't seem to return the correct size on Windows.
                /8;
#else
                ;
#endif
            
            if (grandsize <= 0 || grandsize >= maxsize) {
                    
                progDlg.Update(progbar, "Gapfree file is too large for a single section." \
                               "It will be segmented.\nFile opening may be very slow.");
                
                gapfree=false;
                grandsize = pFH->lNumSamplesPerEpisode / numberChannels;
                finalSections=numberSections;
            }
        }
        Channel TempChannel(finalSections, grandsize);
        Section TempSectionGrand(grandsize, label.str());
        for (int nEpisode=1; nEpisode<=numberSections;++nEpisode) {
            int progbar =
                // Channel contribution:
                (int)(((double)nChannel/(double)numberChannels)*100.0+
                      // Section contribution:
                      (double)(nEpisode-1)/(double)numberSections*(100.0/numberChannels));
            std::ostringstream progStr;
            progStr << "Reading channel #" << nChannel + 1 << " of " << numberChannels
                    << ", Section #" << nEpisode << " of " << numberSections;
            progDlg.Update(progbar, progStr.str());
            
            UINT uNumSamples = 0;
            if (gapfree) {
                if (nEpisode == numberSections) {
                    uNumSamples = grandsize - (nEpisode-1) * pFH->lNumSamplesPerEpisode / numberChannels;
#ifdef _STFDEBUG
                    std::cout << "Last section size " << uNumSamples << std::endl;
#endif
                } else {
                    uNumSamples = pFH->lNumSamplesPerEpisode / numberChannels;
                }
            } else {
                if (!ABF2_GetNumSamples(hFile, pFH, nEpisode, &uNumSamples, &nError)) {
                    std::ostringstream errorMsg;
                    errorMsg << "Exception while calling ABF2_GetNumSamples() "
                             << "for episode # "
                             << nEpisode << "\n"
                             << ABF1Error(fName, nError);
                    ReturnData.resize(0);
                    ABF_Close(hFile,&nError);
                    throw std::runtime_error(errorMsg.str());
                }
            }
            // Use a vector here because memory allocation can
            // be controlled more easily:
            // request memory:
            Vector_float TempSection(uNumSamples, 0.0);
            unsigned int uNumSamplesW;
            if (!ABF2_ReadChannel(hFile, pFH, pFH->nADCSamplingSeq[nChannel],nEpisode,TempSection,
                                  &uNumSamplesW,&nError))
            {
                std::string errorMsg("Exception while calling ABF2_ReadChannel():\n");
                errorMsg += ABF1Error(fName, nError);
                ReturnData.resize(0);
                ABF_Close(hFile,&nError);
                throw std::runtime_error(errorMsg);
            }
            if (uNumSamples!=uNumSamplesW && !gapfree) {
                ABF_Close(hFile,&nError);
                throw std::runtime_error("Exception while calling ABF2_ReadChannel()");
            }
            if (!gapfree) {
                std::ostringstream label;
                label
                    << fName
                    << ", Section # " << nEpisode;
                Section TempSectionT(TempSection.size(),label.str());
                std::copy(TempSection.begin(),TempSection.end(),&TempSectionT[0]);
                try {
                    TempChannel.InsertSection(TempSectionT,nEpisode-1);
                }
                catch (...) {
                    ABF_Close(hFile,&nError);
                    throw;
                }
            } else {
                if ((nEpisode-1) * pFH->lNumSamplesPerEpisode / numberChannels + TempSection.size() <= TempSectionGrand.size()) {
                    std::copy(TempSection.begin(),TempSection.end(),
                              &TempSectionGrand[(nEpisode-1) * pFH->lNumSamplesPerEpisode / numberChannels]);
                }
#ifdef _STFDEBUG
                else {
                    std::cout << "Overflow while copying gapfree sections" << std::endl;
                }
#endif
            }
        }
        if (gapfree) {
            try {
                TempChannel.InsertSection(TempSectionGrand,0);
            }
            catch (...) {
                ABF_Close(hFile,&nError);
                throw;
            }
        }
        try {
            if ((int)ReturnData.size()<numberChannels) {
                ReturnData.resize(numberChannels);
            }
            ReturnData.InsertChannel(TempChannel,nChannel);
        }
        catch (...) {
            ReturnData.resize(0);
            ABF_Close(hFile,&nError);
            throw;
        }
        
        progbar = (int)(((double)(nChannel+1)/(double)numberChannels)*100.0);
        progDlg.Update(progbar, "Completing channel reading\n");

        std::string channel_name( pFH->sADCChannelName[pFH->nADCSamplingSeq[nChannel]] );
        if (channel_name.find("  ")<channel_name.size()) {
            channel_name.erase(channel_name.begin()+channel_name.find("  "),channel_name.end());
        }
        ReturnData[nChannel].SetChannelName(channel_name);

        std::string channel_units( pFH->sADCUnits[pFH->nADCSamplingSeq[nChannel]] );
        if (channel_units.find("  ") < channel_units.size()) {
            channel_units.erase(channel_units.begin() + channel_units.find("  "),channel_units.end());
        }
        ReturnData[nChannel].SetYUnits(channel_units);
    }

    if (!ABF_Close(hFile,&nError)) {
        std::string errorMsg("Exception in importABFFile():\n");
        errorMsg += ABF1Error(fName,nError);
        ReturnData.resize(0);
        throw std::runtime_error(errorMsg);
    }
    
    ReturnData.SetXScale((double)(pFH->fADCSequenceInterval/1000.0));
    
    std::string comment("Created with ");
    comment += std::string( pFH->sCreatorInfo );
    ReturnData.SetComment(comment);
    ReturnData.SetDate(dateToStr(pFH->uFileStartDate));
    ReturnData.SetTime(timeToStr(pFH->uFileStartTimeMS));

    abf2.Close();
}

void stfio::importABF1File(const std::string &fName, Recording &ReturnData, ProgressInfo& progDlg) {
    
    int hFile = 0;
    ABFFileHeader FH;
    UINT uMaxSamples = 0;
    DWORD dwMaxEpi = 0;
    int nError = 0;

    std::wstring wfName;

    for(std::string::size_type i=0; i<fName.size(); ++i) {
        wfName += wchar_t(fName[i]);
    }

    if (!ABF_ReadOpen(wfName.c_str(), &hFile, ABF_DATAFILE, &FH,
                      &uMaxSamples, &dwMaxEpi, &nError))
    {
        std::string errorMsg("Exception while calling ABF_ReadOpen():\n");
        errorMsg+=ABF1Error(fName,nError);
        ABF_Close(hFile,&nError);
        throw std::runtime_error(errorMsg);
    }
    /*	if (!ABF_HasData(hFile,pFH)) {
    std::string errorMsg("Exception while calling ABF_ReadOpen():\n"
    "File is empty");
    throw std::runtime_error(errorMsg);
    }
    */
    int numberChannels=FH.nADCNumChannels;
    ABFLONG numberSections=FH.lActualEpisodes;
    if ((DWORD)numberSections>dwMaxEpi) {
        ABF_Close(hFile,&nError);
        throw std::runtime_error("Error while calling stfio::importABFFile():\n"
            "lActualEpisodes>dwMaxEpi");
    }
    for (int nChannel=0;nChannel<numberChannels;++nChannel) {
        Channel TempChannel(numberSections);
        for (DWORD dwEpisode=1;dwEpisode<=(DWORD)numberSections;++dwEpisode) {
            int progbar = // Channel contribution:
                (int)(((double)nChannel/(double)numberChannels)*100.0+
                      // Section contribution:
                      (double)(dwEpisode-1)/(double)numberSections*(100.0/numberChannels));
            std::ostringstream progStr;
            progStr << "Reading channel #" << nChannel + 1 << " of " << numberChannels
                    << ", Section #" << dwEpisode << " of " << numberSections;
            progDlg.Update(progbar, progStr.str());
            
            unsigned int uNumSamples=0;
            if (!ABF_GetNumSamples(hFile,&FH,dwEpisode,&uNumSamples,&nError)) {
                std::string errorMsg( "Exception while calling ABF_GetNumSamples():\n" );
                errorMsg += ABF1Error(fName, nError);
                ReturnData.resize(0);
                ABF_Close(hFile,&nError);
                throw std::runtime_error(errorMsg);
            }
            // Use a vector here because memory allocation can
            // be controlled more easily:
            // request memory:
            Vector_float TempSection(uNumSamples, 0.0);
            unsigned int uNumSamplesW=0;
            if (!ABF_ReadChannel(hFile, &FH, FH.nADCSamplingSeq[nChannel], dwEpisode, TempSection,
                                 &uNumSamplesW, &nError))
            {
                std::string errorMsg("Exception while calling ABF_ReadChannel():\n");
                errorMsg += ABF1Error(fName, nError);
                ReturnData.resize(0);
                ABF_Close(hFile,&nError);
                throw std::runtime_error(errorMsg);
            }
            if (uNumSamples!=uNumSamplesW) {
                ABF_Close(hFile,&nError);
                throw std::runtime_error("Exception while calling ABF_ReadChannel()");
            }
            std::ostringstream label;
            label
                << fName
                << ", Section # " << dwEpisode;
            Section TempSectionT(TempSection.size(),label.str());
            std::copy(TempSection.begin(),TempSection.end(),&TempSectionT[0]);
            try {
                TempChannel.InsertSection(TempSectionT,dwEpisode-1);
            }
            catch (...) {
                ABF_Close(hFile,&nError);
                throw;
            }
        }
        try {
            if ((int)ReturnData.size()<numberChannels) {
                ReturnData.resize(numberChannels);
            }
            ReturnData.InsertChannel(TempChannel,nChannel);
        }
        catch (...) {
            ReturnData.resize(0);
            ABF_Close(hFile,&nError);
            throw;
        }

        std::string channel_name( FH.sADCChannelName[FH.nADCSamplingSeq[nChannel]] );
        if (channel_name.find("  ")<channel_name.size()) {
            channel_name.erase(channel_name.begin()+channel_name.find("  "),channel_name.end());
        }
        ReturnData[nChannel].SetChannelName(channel_name);

        std::string channel_units( FH.sADCUnits[FH.nADCSamplingSeq[nChannel]] );
        if (channel_units.find("  ") < channel_units.size()) {
            channel_units.erase(channel_units.begin() + channel_units.find("  "),channel_units.end());
        }
        ReturnData[nChannel].SetYUnits(channel_units);
    }

    if (!ABF_Close(hFile,&nError)) {
        std::string errorMsg("Exception in importABFFile():\n");
        errorMsg += ABF1Error(fName,nError);
        ReturnData.resize(0);
        throw std::runtime_error(errorMsg);
    }
    // Apparently, the sample interval has to be multiplied by
    // the number of channels for multiplexed data. Thanks to
    // Dominique Engel for noticing.
    ReturnData.SetXScale((double)(FH.fADCSampleInterval/1000.0)*(double)numberChannels);
    std::string comment("Created with ");
    comment += std::string( FH.sCreatorInfo );
    ReturnData.SetComment(comment);
    ReturnData.SetDate(dateToStr(FH.lFileStartDate));
    ReturnData.SetTime(timeToStr(FH.lFileStartTime));
    
}
#ifdef _WINDOWS
#pragma optimize ("", on)
#endif