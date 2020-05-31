/**********************************************************************************************************************
 *
 * Epson TM-T88V Printer Driver for GNU/Linux
 *
 * Copyright (C) Seiko Epson Corporation 2019.
 * Copyright (C) 2020 Grégory DAVID.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *********************************************************************************************************************/
#include <cups/ppd.h>
#include <cups/raster.h>

#include <cstdint>
#include <csignal>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <limits.h> // LONG_MAX

/*------------
 * Result code
 *------------*/
#define EPTMD_SUCCESS (0) // Processing succeeded.
#define EPTMD_FAILED (1) // Processing failed.
#define EPTMD_CANCEL (2) // Processing canceled.

/*--------------------
 * command declaration
 *--------------------*/
#define ESC (0x1b)
#define GS (0x1d)

/*----------------
 * MACRO (#define)
 *----------------*/
#define EPTMD_BITS_TO_BYTES(bits) (((bits) + 7) / 8)

/*-----------------
 * enum declaration
 *-----------------*/
typedef enum
{
  TmPaperReductionOff = 0,
  TmPaperReductionTop,
  TmPaperReductionBottom,
  TmPaperReductionBoth,
} EPTME_BLANK_SKIP_TYPE; // Paper Reduction

typedef enum
{
  TmBuzzerNotUsed = 0,
  TmBuzzerInternal,
  TmBuzzerExternal,
} EPTME_BUZZER; // Buzzer Type

typedef enum
{
  TmDrawerNotUsed = 0,
  TmDrawer1,
  TmDrawer2,
} EPTME_DRAWER; // Drawer No

typedef enum
{
  TmNoCut = 0,
  TmCutPerJob,
  TmCutPerPage,
} EPTME_PAPER_CUT; // Paper Cut

/*--------------------------------
 * Structure prototype declaration
 *--------------------------------*/
typedef struct
{
  char *p_printerName; // The name of the destination printer.
  unsigned char h_motionUnit; // Horizontal motion units.
  unsigned char v_motionUnit; // Vertical motion units.
  EPTME_BLANK_SKIP_TYPE paperReduction; // Paper reduction settings.
  EPTME_BUZZER buzzerControl; // Buzzer control settings.
  EPTME_DRAWER drawerControl; // Drawer control settings.
  EPTME_PAPER_CUT cutControl; // Paper cut settings.
  unsigned maxBandLines; // Maximum band length.
} EPTMS_CONFIG_T; // Configuration parameters

typedef struct
{
  cups_raster_t *p_raster;
  cups_page_header2_t pageHeader;
  unsigned char *p_pageBuffer;
} EPTMS_JOB_INFO_T; // Job Information parameters

using result_t = std::uint16_t;

/*----------------------------
 * Global variable declaration
 *----------------------------*/
char g_TmCanceled;

/*--------------------------------------
 * Static function prototype declaration
 *--------------------------------------*/
static void fprintf_DebugLog(EPTMS_CONFIG_T *);
static result_t Init(int, char *[], EPTMS_CONFIG_T *, EPTMS_JOB_INFO_T *, int *);
static result_t InitSignal(void);
static void SignalCallback(int);
static result_t GetParameters(char *[], EPTMS_CONFIG_T *);
static result_t GetModelSpecificFromPPD(ppd_file_t *, EPTMS_CONFIG_T *);
static result_t GetPaperReductionFromPPD(ppd_file_t *, EPTMS_CONFIG_T *);
static result_t GetBuzzerAndDrawerFromPPD(ppd_file_t *, EPTMS_CONFIG_T *);
static result_t GetPaperCutFromPPD(ppd_file_t *, EPTMS_CONFIG_T *);
static void Exit(EPTMS_JOB_INFO_T *, int *);

static result_t DoJob(EPTMS_CONFIG_T *, EPTMS_JOB_INFO_T *);
static result_t StartJob(EPTMS_CONFIG_T *, EPTMS_JOB_INFO_T *);
static result_t OpenDrawer(EPTMS_CONFIG_T *);
static result_t SoundBuzzer(EPTMS_CONFIG_T *);
static result_t EndJob(EPTMS_CONFIG_T *, EPTMS_JOB_INFO_T *, cups_page_header2_t *);

static result_t DoPage(EPTMS_CONFIG_T *, EPTMS_JOB_INFO_T *);
static result_t StartPage(EPTMS_CONFIG_T *);
static result_t EndPage(EPTMS_CONFIG_T *, cups_page_header2_t *);
static result_t ReadRaster(cups_page_header2_t *, cups_raster_t *, unsigned char *);
static void TransferRaster(unsigned char *, unsigned char *, cups_page_header2_t *, unsigned);
static result_t WriteRaster(EPTMS_CONFIG_T *, cups_page_header2_t *, unsigned char *);
static void AvoidDisturbingData(cups_page_header2_t *, unsigned char *, unsigned, unsigned);
static unsigned FindBlackRasterLineTop(cups_page_header2_t *, unsigned char *);
static unsigned FindBlackRasterLineEnd(cups_page_header2_t *, unsigned char *);
static result_t WriteBand(cups_page_header2_t *, unsigned char *, unsigned);

static result_t WriteUserFile(char *, const char *);
static unsigned int ReadUserFile(int, void *, unsigned int);
static result_t WriteData(unsigned char *, unsigned int);

int main(int argc, char **argv)
{
  EPTMS_JOB_INFO_T JobInfo = {0};
  EPTMS_CONFIG_T Config = {0};
  int InputFd = -1;
  result_t result = EPTMD_SUCCESS;
  // Initializes process.
  result = Init(argc, argv, &Config, &JobInfo, &InputFd);

  // Processing print job.
  if(EPTMD_SUCCESS == result)
  {
    result = DoJob(&Config, &JobInfo);
  }

  // Finalizes process.
  Exit(&JobInfo, &InputFd);

  // Error log output.
  if(EPTMD_SUCCESS != result)
  {
    fprintf(stderr, "ERROR: Error Code=%d\n", result);
  }

  // Output message for debugging.
  fprintf_DebugLog(&Config);
  return result;
}

static void fprintf_DebugLog(EPTMS_CONFIG_T *p_config)
{
  fprintf(stderr, "DEBUG: p_printerName = %s\n", p_config->p_printerName);
  fprintf(stderr, "DEBUG: v_motionUnit = %u\n", p_config->v_motionUnit);
  fprintf(stderr, "DEBUG: h_motionUnit = %u\n", p_config->h_motionUnit);
  fprintf(stderr, "DEBUG: paperReduction = %d\n", p_config->paperReduction);
  fprintf(stderr, "DEBUG: buzzerControl = %d\n", p_config->buzzerControl);
  fprintf(stderr, "DEBUG: drawerControl = %d\n", p_config->drawerControl);
  fprintf(stderr, "DEBUG: cutControl = %d\n", p_config->cutControl);
  fprintf(stderr, "DEBUG: maxBandLines = %u\n", p_config->maxBandLines);
}

static result_t Init(int argc, char *argv[],
                     EPTMS_CONFIG_T *p_config,
                     EPTMS_JOB_INFO_T *p_jobInfo,
                     int *p_InputFd)
{
  result_t result = EPTMD_SUCCESS;
  // Initializes global variables.
  g_TmCanceled = 0;

  // Check parameters.
  if((nullptr == argv) || ((6 != argc) && (7 != argc)))
  {
    return 1001;
  }

  // Initializes signals.
  result = InitSignal();

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  {
    // Open a raster stream.
    if(6 == argc)
    {
      *p_InputFd = 0;
    }
    else if(7 == argc)
    {
      *p_InputFd = open(argv[6], O_RDONLY);

      if(*p_InputFd < 0)
      {
        return 1002;
      }
    }
    else {}

    p_jobInfo->p_raster = cupsRasterOpen(*p_InputFd, CUPS_RASTER_READ);

    if(nullptr == p_jobInfo->p_raster)
    {
      return 1003;
    }
  }

  // Get parameters.
  result = GetParameters(argv, p_config);

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  // Get printer name.
  p_config->p_printerName = argv[0];
  p_config->maxBandLines = 256;
  return EPTMD_SUCCESS;
}

static result_t InitSignal(void)
{
  sigset_t sigset;
  {
    //
    if(0 != sigemptyset(&sigset))
    {
      return 1101;
    }

    if(0 != sigaddset(&sigset, SIGTERM))
    {
      return 1102;
    }

    if(0 != sigprocmask(SIG_BLOCK, &sigset, nullptr))
    {
      return 1103;
    }
  }
  {
    //
    struct sigaction sigact_term;

    if(0 != sigaction(SIGTERM, nullptr, &sigact_term))
    {
      return 1104;
    }

    sigact_term.sa_handler = SignalCallback;
    sigact_term.sa_flags |= SA_RESTART;

    if(0 != sigaction(SIGTERM, &sigact_term, nullptr))
    {
      return 1105;
    }
  }

  if(0 != sigprocmask(SIG_UNBLOCK, &sigset, nullptr))
  {
    return 1106;
  }

  return EPTMD_SUCCESS;
}

static void SignalCallback(int signal_id)
{
  (void)signal_id; // unused parameter
  g_TmCanceled = 1;
}

static result_t GetParameters(char *argv[], EPTMS_CONFIG_T *p_config)
{
  ppd_file_t *p_ppd = nullptr;
  {
    // Load the PPD file
    p_ppd = ppdOpenFile(getenv("PPD"));

    if(nullptr == p_ppd)
    {
      return 4001;
    }

    ppdMarkDefaults(p_ppd);
  }
  {
    // Check conflict
    cups_option_t *p_options = nullptr;
    int num_option = cupsParseOptions(argv[5], 0, &p_options);

    if(0 < num_option)
    {
      if(0 != cupsMarkOptions(p_ppd, num_option, p_options)) // 1 if conflicts exist, 0 otherwise
      {
        ppdClose(p_ppd);
        cupsFreeOptions(num_option, p_options);
        return 4002;
      }
    }

    cupsFreeOptions(num_option, p_options);
  }
  result_t result = EPTMD_SUCCESS;
  {
    // Get parameters
    if(EPTMD_SUCCESS == result)
    {
      result = GetModelSpecificFromPPD(p_ppd, p_config);
    }

    if(EPTMD_SUCCESS == result)
    {
      result = GetPaperReductionFromPPD(p_ppd, p_config);
    }

    if(EPTMD_SUCCESS == result)
    {
      result = GetPaperCutFromPPD(p_ppd, p_config);
    }

    if(EPTMD_SUCCESS == result)
    {
      result = GetBuzzerAndDrawerFromPPD(p_ppd, p_config);
    }
  }
  // Unload the PPD file
  ppdClose(p_ppd);
  return result;
}

static result_t GetModelSpecificFromPPD(ppd_file_t *p_ppd, EPTMS_CONFIG_T *p_config)
{
  {
    char ppdKeyMotionUnit[] = "TmxMotionUnitHori";
    ppd_attr_t *p_attribute = ppdFindAttr(p_ppd, ppdKeyMotionUnit, nullptr);

    if(nullptr == p_attribute)
    {
      return 4101;
    }

    p_config->h_motionUnit = (unsigned char)atol(p_attribute->value);

    if((0 == p_config->h_motionUnit) || (255 < p_config->h_motionUnit)) // GS P Command Specification
    {
      return 4102;
    }
  }
  {
    char ppdKeyMotionUnit[] = "TmxMotionUnitVert";
    ppd_attr_t *p_attribute = ppdFindAttr(p_ppd, ppdKeyMotionUnit, nullptr);

    if(nullptr == p_attribute)
    {
      return 4103;
    }

    p_config->v_motionUnit = (unsigned char)atol(p_attribute->value);

    if((0 == p_config->v_motionUnit) || (255 < p_config->v_motionUnit)) // GS P Command Specification
    {
      return 4104;
    }
  }
  return EPTMD_SUCCESS;
}

static result_t GetPaperReductionFromPPD(ppd_file_t *p_ppd, EPTMS_CONFIG_T *p_config)
{
  char ppdKey[] = "TmxPaperReduction";
  ppd_choice_t *p_choice = ppdFindMarkedChoice(p_ppd, ppdKey);

  if(nullptr == p_choice)
  {
    return 4201;
  }

  if(0 == strcmp("Off", p_choice->choice))
  {
    p_config->paperReduction = TmPaperReductionOff;
  }
  else if(0 == strcmp("Top", p_choice->choice))
  {
    p_config->paperReduction = TmPaperReductionTop;
  }
  else if(0 == strcmp("Bottom", p_choice->choice))
  {
    p_config->paperReduction = TmPaperReductionBottom;
  }
  else if(0 == strcmp("Both", p_choice->choice))
  {
    p_config->paperReduction = TmPaperReductionBoth;
  }
  else
  {
    return 4202;
  }

  return EPTMD_SUCCESS;
}

static result_t GetBuzzerAndDrawerFromPPD(ppd_file_t *p_ppd, EPTMS_CONFIG_T *p_config)
{
  char ppdKey[] = "TmxBuzzerAndDrawer";
  ppd_choice_t *p_choice = ppdFindMarkedChoice(p_ppd, ppdKey);

  if(nullptr == p_choice)
  {
    return 4301;
  }

  if(0 == strcmp("NotUsed", p_choice->choice))
  {
    p_config->buzzerControl = TmBuzzerNotUsed;
    p_config->drawerControl = TmDrawerNotUsed;
  }
  else if(0 == strcmp("InternalBuzzer", p_choice->choice))
  {
    p_config->buzzerControl = TmBuzzerInternal;
  }
  else if(0 == strcmp("ExternalBuzzer", p_choice->choice))
  {
    p_config->buzzerControl = TmBuzzerExternal;
  }
  else if(0 == strcmp("OpenDrawer1", p_choice->choice))
  {
    p_config->drawerControl = TmDrawer1;
  }
  else if(0 == strcmp("OpenDrawer2", p_choice->choice))
  {
    p_config->drawerControl = TmDrawer2;
  }
  else
  {
    return 4302;
  }

  return EPTMD_SUCCESS;
}

static result_t GetPaperCutFromPPD(ppd_file_t *p_ppd, EPTMS_CONFIG_T *p_config)
{
  char ppdKey[] = "TmxPaperCut";
  ppd_choice_t *p_choice = ppdFindMarkedChoice(p_ppd, ppdKey);

  if(nullptr == p_choice)
  {
    return 4401;
  }

  if(0 == strcmp("NoCut", p_choice->choice))
  {
    p_config->cutControl = TmNoCut;
  }
  else if(0 == strcmp("CutPerJob", p_choice->choice))
  {
    p_config->cutControl = TmCutPerJob;
  }
  else if(0 == strcmp("CutPerPage", p_choice->choice))
  {
    p_config->cutControl = TmCutPerPage;
  }
  else
  {
    return 4402;
  }

  return EPTMD_SUCCESS;
}

static void Exit(EPTMS_JOB_INFO_T *p_jobInfo, int *p_InputFd)
{
  if(nullptr != p_jobInfo->p_raster)
  {
    cupsRasterClose(p_jobInfo->p_raster);
    p_jobInfo->p_raster = nullptr;
  }

  if(0 < *p_InputFd)
  {
    close(*p_InputFd);
    *p_InputFd = -1;
  }
}

static result_t DoJob(EPTMS_CONFIG_T *p_config, EPTMS_JOB_INFO_T *p_jobInfo)
{
  result_t result = EPTMD_SUCCESS;
  unsigned page = 0;
  result = StartJob(p_config, p_jobInfo);

  while(EPTMD_SUCCESS == result)
  {
    if(0 == cupsRasterReadHeader2(p_jobInfo->p_raster, &p_jobInfo->pageHeader))
    {
      result = EPTMD_SUCCESS;
      break;
    }

    page++;
    fprintf(stderr, "PAGE: %u %d\n", page, p_jobInfo->pageHeader.NumCopies);
    fprintf(stderr, "DEBUG: cupsBytesPerLine = %u\n", p_jobInfo->pageHeader.cupsBytesPerLine);
    fprintf(stderr, "DEBUG: cupsBitsPerPixel = %u\n", p_jobInfo->pageHeader.cupsBitsPerPixel);
    fprintf(stderr, "DEBUG: cupsBitsPerColor = %u\n", p_jobInfo->pageHeader.cupsBitsPerColor);
    fprintf(stderr, "DEBUG: cupsHeight = %u\n", p_jobInfo->pageHeader.cupsHeight);
    fprintf(stderr, "DEBUG: cupsWidth = %u\n", p_jobInfo->pageHeader.cupsWidth);

    if(1 != p_jobInfo->pageHeader.cupsBitsPerPixel)
    {
      result = 2001;
      break;
    }

    if(nullptr == p_jobInfo->p_pageBuffer) // Allocate buffer of page.
    {
      std::size_t size = p_jobInfo->pageHeader.cupsHeight * EPTMD_BITS_TO_BYTES(p_jobInfo->pageHeader.cupsWidth);
      p_jobInfo->p_pageBuffer = (unsigned char *)malloc(size);

      if(nullptr == p_jobInfo->p_pageBuffer)
      {
        result = 2002;
        break;
      }

      memset(p_jobInfo->p_pageBuffer, 0, size);
    }

    result = DoPage(p_config, p_jobInfo);
  }

  // Free buffer of page.
  if(nullptr != p_jobInfo->p_pageBuffer)
  {
    free(p_jobInfo->p_pageBuffer);
    p_jobInfo->p_pageBuffer = nullptr;
  }

  if(EPTMD_SUCCESS != result)
  {
    EndJob(p_config, p_jobInfo, &p_jobInfo->pageHeader);
  }
  else
  {
    result = EndJob(p_config, p_jobInfo, &p_jobInfo->pageHeader);
  }

  return result;
}

static result_t StartJob(EPTMS_CONFIG_T *p_config,
                         EPTMS_JOB_INFO_T *)
{
  result_t result = EPTMD_SUCCESS;

  if(0 != g_TmCanceled)
  {
    return EPTMD_CANCEL;
  }

  {
    // Write configuration commands.
    unsigned char CommandSetDevice[3 + 2] = { ESC, '=', 0x01, ESC, '@' };
    result = WriteData(CommandSetDevice, sizeof(CommandSetDevice));

    if(EPTMD_SUCCESS != result)
    {
      return 2101;
    }

    unsigned char CommandSetPrintSheet[4] = { ESC, 'c', '0', 0x02 };
    result = WriteData(CommandSetPrintSheet, sizeof(CommandSetPrintSheet));

    if(EPTMD_SUCCESS != result)
    {
      return 2102;
    }

    unsigned char CommandSetConfigSheet[4] = { ESC, 'c', '1', 0x02 };
    result = WriteData(CommandSetConfigSheet, sizeof(CommandSetConfigSheet));

    if(EPTMD_SUCCESS != result)
    {
      return 2103;
    }

    unsigned char CommandSetNearendPrint[4] = { ESC, 'c', '3', 0x00 };
    result = WriteData(CommandSetNearendPrint, sizeof(CommandSetNearendPrint));

    if(EPTMD_SUCCESS != result)
    {
      return 2104;
    }

    unsigned char CommandSetBaseMotionUnit[4] = { GS, 'P', 0x00, 0x00 };
    CommandSetBaseMotionUnit[2] = p_config->h_motionUnit & 0xff;
    CommandSetBaseMotionUnit[3] = p_config->v_motionUnit;
    result = WriteData(CommandSetBaseMotionUnit, sizeof(CommandSetBaseMotionUnit));

    if(EPTMD_SUCCESS != result)
    {
      return 2105;
    }
  }

  // Drawer open.
  result = OpenDrawer(p_config);

  if(EPTMD_SUCCESS != result)
  {
    return 2106;
  }

  // Sound buzzer.
  result = SoundBuzzer(p_config);

  if(EPTMD_SUCCESS != result)
  {
    return 2107;
  }

  // Send user file.
  result = WriteUserFile(p_config->p_printerName, "StartJob.prn");

  if(EPTMD_SUCCESS != result)
  {
    return 2108;
  }

  return EPTMD_SUCCESS;
}

static result_t OpenDrawer(EPTMS_CONFIG_T *p_config)
{
  result_t result = EPTMD_SUCCESS;

  if(TmDrawerNotUsed == p_config->drawerControl)
  {
    return EPTMD_SUCCESS;
  }

  unsigned char Command[5] = { ESC, 'p', 0, 50 /* on time */, 200 /* off time */ };
  Command[2] = static_cast<unsigned char>(p_config->drawerControl - 1); // pin no
  result = WriteData(Command, sizeof(Command));
  return result;
}

static result_t SoundBuzzer(EPTMS_CONFIG_T *p_config)
{
  result_t result = EPTMD_SUCCESS;

  if(TmBuzzerNotUsed == p_config->buzzerControl)
  {
    return EPTMD_SUCCESS;
  }
  else if(TmBuzzerInternal == p_config->buzzerControl) // Sound internal buzzer
  {
    unsigned char Command[5] = { ESC, 'p', 1/* pin no */, 50/* on time */, 200/* off time */ };
    int n;

    for(n = 0; n < 1 /* repeat count */; n++)
    {
      result = WriteData(Command, sizeof(Command));

      if(EPTMD_SUCCESS != result)
      {
        return result;
      }
    }
  }
  else if(TmBuzzerExternal == p_config->buzzerControl) // Sound external buzzer
  {
    unsigned char Command[10] = { ESC, '(', 'A', 5, 0, 97, 100, 1, 50/* on time */, 200/* off time */ };
    result = WriteData(Command, sizeof(Command));

    if(EPTMD_SUCCESS != result)
    {
      return result;
    }
  }
  else {}

  return EPTMD_SUCCESS;
}

static result_t EndJob(EPTMS_CONFIG_T *p_config,
                       EPTMS_JOB_INFO_T *,
                       cups_page_header2_t *)
{
  result_t result = EPTMD_SUCCESS;

  if(0 != g_TmCanceled)
  {
    return EPTMD_CANCEL;
  }

  // Send user file.
  result = WriteUserFile(p_config->p_printerName, "EndJob.prn");

  if(EPTMD_SUCCESS != result)
  {
    return 2201;
  }

  // Feed and cut paper.
  unsigned char Command[3 + 4] = { ESC, 'J', 0, GS, 'V', 66, 0 };

  switch(p_config->cutControl)
  {
    case TmCutPerJob:
      result = WriteData(Command, sizeof(Command));

      if(EPTMD_SUCCESS != result)
      {
        return 2202;
      }

      break;

    default:
      break;
  }

  return EPTMD_SUCCESS;
}

static result_t DoPage(EPTMS_CONFIG_T *p_config, EPTMS_JOB_INFO_T *p_jobInfo)
{
  result_t result;
  result = StartPage(p_config);

  if(EPTMD_SUCCESS == result)
  {
    result = ReadRaster(&p_jobInfo->pageHeader, p_jobInfo->p_raster, p_jobInfo->p_pageBuffer);
  }

  if(EPTMD_SUCCESS == result)
  {
    result = WriteRaster(p_config, &p_jobInfo->pageHeader, p_jobInfo->p_pageBuffer);
  }

  if(EPTMD_SUCCESS == result)
  {
    result = EndPage(p_config, &p_jobInfo->pageHeader);
  }

  return result;
}

static result_t StartPage(EPTMS_CONFIG_T *p_config)
{
  int result;
  // Send user file.
  result = WriteUserFile(p_config->p_printerName, "StartPage.prn");

  if(EPTMD_SUCCESS != result)
  {
    return 3102;
  }

  return EPTMD_SUCCESS;
}

static result_t EndPage(EPTMS_CONFIG_T *p_config,
                        cups_page_header2_t *)
{
  int result;

  if(0 != g_TmCanceled)
  {
    return EPTMD_CANCEL;
  }

  // Send user file.
  result = WriteUserFile(p_config->p_printerName, "EndPage.prn");

  if(EPTMD_SUCCESS != result)
  {
    return 3201;
  }

  // Feed and cut paper.
  unsigned char Command[3 + 4] = { ESC, 'J', 0, GS, 'V', 66, 0 };

  switch(p_config->cutControl)
  {
    case TmCutPerPage:
      result = WriteData(Command, sizeof(Command));

      if(EPTMD_SUCCESS != result)
      {
        return 3202;
      }

      break;

    default:
      break;
  }

  return EPTMD_SUCCESS;
}

static result_t ReadRaster(cups_page_header2_t *p_header, cups_raster_t *p_raster, unsigned char *p_pageBuffer)
{
  result_t result = EPTMD_SUCCESS;
  unsigned char *p_data;
  unsigned data_size = p_header->cupsBytesPerLine;
  p_data = (unsigned char *)malloc(data_size);

  if(nullptr == p_data)
  {
    return 3301;
  }

  memset(p_data, 0, data_size);
  unsigned i;

  for(i = 0; i < p_header->cupsHeight; i++)
  {
    if(0 != g_TmCanceled)
    {
      result = EPTMD_CANCEL;
      break;
    }

    unsigned num_bytes_read = cupsRasterReadPixels(p_raster, p_data, data_size);

    if(data_size > num_bytes_read)
    {
      fprintf(stderr, "DEBUG: cupsRasterReadPixels() = %u:%u/%u\n", (i + 1), num_bytes_read, data_size);
      result = 3302;
      break;
    }

    TransferRaster(p_pageBuffer, p_data, p_header, i);
  }

  free(p_data);
  return result;
}

static void TransferRaster(unsigned char *p_pageBuffer, unsigned char *p_data, cups_page_header2_t *p_header, unsigned line_no)
{
  unsigned char *p_dest = p_pageBuffer + (EPTMD_BITS_TO_BYTES(p_header->cupsWidth) * line_no);
  memcpy(p_dest, p_data, p_header->cupsBytesPerLine);
}

static result_t WriteRaster(EPTMS_CONFIG_T *p_config, cups_page_header2_t *p_header, unsigned char *p_pageBuffer)
{
  unsigned line_no = 0;
  unsigned start_line_no = 0; /* first raster line without top blank */
  unsigned last_line_no = 0; /* last raster line without bottom blank */
  unsigned char *p_data = nullptr;
  result_t result = EPTMD_SUCCESS;
  // Get top margin
  start_line_no = FindBlackRasterLineTop(p_header, p_pageBuffer);

  if(p_header->cupsHeight == start_line_no) /* This page has not image */
  {
    return EPTMD_SUCCESS;
  }

  // Get bottom margin
  last_line_no = FindBlackRasterLineEnd(p_header, p_pageBuffer) + 1;
  // Avoid disturbing data
  AvoidDisturbingData(p_header, p_pageBuffer, start_line_no, last_line_no);

  // Command output : raster data (band unit)
  for(line_no = start_line_no; (line_no + p_config->maxBandLines) < last_line_no; line_no += p_config->maxBandLines)
  {
    p_data = p_pageBuffer + (EPTMD_BITS_TO_BYTES(p_header->cupsWidth) * line_no);
    result = WriteBand(p_header, p_data, p_config->maxBandLines);

    if(EPTMD_SUCCESS != result)
    {
      return 3403;
    }

    if(0 != g_TmCanceled)
    {
      return EPTMD_CANCEL;
    }
  }

  // Command output : raster data
  if(line_no < last_line_no)
  {
    p_data = p_pageBuffer + (EPTMD_BITS_TO_BYTES(p_header->cupsWidth) * line_no);
    result = WriteBand(p_header, p_data, (last_line_no - line_no));

    if(EPTMD_SUCCESS != result)
    {
      return 3404;
    }
  }

  return EPTMD_SUCCESS;
}

static void AvoidDisturbingData(cups_page_header2_t *p_header, unsigned char *p_pageBuffer, unsigned start_line_no, unsigned last_line_no)
{
  unsigned char *p_data = p_pageBuffer + (EPTMD_BITS_TO_BYTES(p_header->cupsWidth) * start_line_no);
  unsigned long data_size = (last_line_no - start_line_no) * EPTMD_BITS_TO_BYTES(p_header->cupsWidth);
  unsigned long i = 0;

  for(; (i + 1) < data_size; i++)
  {
    if(0x10 == p_data[i])
    {
      if((0x04 == p_data[i + 1]) || (0x05 == p_data[i + 1]) || (0x14 == p_data[i + 1]))
      {
        p_data[i] = 0x30;
      }
    }
    else if(0x1B == p_data[i])
    {
      if(0x3D == p_data[i + 1])
      {
        p_data[i] = 0x3B;
      }
    }
    else {}
  }
}

static unsigned FindBlackRasterLineTop(cups_page_header2_t *p_header, unsigned char *p_pageBuffer)
{
  unsigned BytesPerLine = EPTMD_BITS_TO_BYTES(p_header->cupsWidth);
  unsigned char *p_data = p_pageBuffer;
  unsigned y;

  for(y = 0; y < p_header->cupsHeight; y++)
  {
    unsigned x;

    for(x = 0 ; x < BytesPerLine; x++)
    {
      if(0x00 != p_data[x])
      {
        return y;
      }
    }

    p_data += BytesPerLine;
  }

  return p_header->cupsHeight;
}

static unsigned FindBlackRasterLineEnd(cups_page_header2_t *p_header, unsigned char *p_pageBuffer)
{
  unsigned BytesPerLine = EPTMD_BITS_TO_BYTES(p_header->cupsWidth);
  unsigned char *p_data = p_pageBuffer + (BytesPerLine * (p_header->cupsHeight - 1));
  unsigned y;

  for(y = 0; y < p_header->cupsHeight; y++)
  {
    unsigned x;

    for(x = 0 ; x < BytesPerLine; x++)
    {
      if(0x00 != p_data[x])
      {
        return (p_header->cupsHeight - 1 - y);
      }
    }

    p_data -= BytesPerLine;
  }

  return 0;
}

static result_t WriteBand(cups_page_header2_t *p_header, unsigned char *p_data, unsigned lines)
{
  result_t result = EPTMD_SUCCESS;
  unsigned char CommandSetAbsolutePrintPosition[4] = { ESC, '$', 0, 0 };
  result = WriteData(CommandSetAbsolutePrintPosition, sizeof(CommandSetAbsolutePrintPosition));

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  unsigned long width = p_header->cupsWidth;
  unsigned char CommandSetGraphicsdataGS8L112[17] = { GS, '8', 'L', 0, 0, 0, 0, 48, 112, 48, 1, 1, 49, 0, 0, 0, 0 };
  CommandSetGraphicsdataGS8L112[3] = (unsigned char)(((EPTMD_BITS_TO_BYTES(width) * lines) + 10)) & 0xff;
  CommandSetGraphicsdataGS8L112[4] = (unsigned char)(((EPTMD_BITS_TO_BYTES(width) * lines) + 10) >> 8) & 0xff;
  CommandSetGraphicsdataGS8L112[5] = (unsigned char)(((EPTMD_BITS_TO_BYTES(width) * lines) + 10) >> 16) & 0xff;
  CommandSetGraphicsdataGS8L112[6] = (unsigned char)(((EPTMD_BITS_TO_BYTES(width) * lines) + 10) >> 24) & 0xff;
  CommandSetGraphicsdataGS8L112[13] = (unsigned char)((width) & 0xff);
  CommandSetGraphicsdataGS8L112[14] = (unsigned char)((width >> 8) & 0xff);
  CommandSetGraphicsdataGS8L112[15] = (unsigned char)((lines) & 0xff);
  CommandSetGraphicsdataGS8L112[16] = (unsigned char)((lines >> 8) & 0xff);
  result = WriteData(CommandSetGraphicsdataGS8L112, sizeof(CommandSetGraphicsdataGS8L112));

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  result = WriteData(p_data, (unsigned int)(EPTMD_BITS_TO_BYTES(width) * lines));

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  unsigned char CommandSetGraphicsdataGSpL50[7] = { GS, '(', 'L', 2, 0, 48, 50 };
  result = WriteData(CommandSetGraphicsdataGSpL50, sizeof(CommandSetGraphicsdataGSpL50));

  if(EPTMD_SUCCESS != result)
  {
    return result;
  }

  return EPTMD_SUCCESS;
}

static result_t WriteUserFile(char *p_printerName, const char *p_file_name)
{
  result_t result = EPTMD_SUCCESS;
  // Output a file if it exists in a predetermined place. : /var/lib/tmx-cups
  std::ostringstream path;
  std::string os_specific_dirname;
#ifndef EPD_TM_MAC
  os_specific_dirname = "/var/lib/tmx-cups";
#else
  os_specific_dirname = "/Library/Caches/Epson/TerminalPrinter";
#endif
  path << os_specific_dirname << p_printerName << "_" << p_file_name;
  int fd = open(path.str().c_str(), O_RDONLY);

  if(0 > fd)
  {
    if(ENOENT == errno) // No such file or directory
    {
      return EPTMD_SUCCESS;
    }

    return EPTMD_FAILED;
  }

  while(1)
  {
    char data[1024] = { 0 };
    unsigned int size = ReadUserFile(fd, data, sizeof(data));
    result = WriteData((unsigned char *)data, size);

    if(EPTMD_SUCCESS != result)
    {
      close(fd);
      return EPTMD_FAILED;
    }
  }

  if(close(fd) < 0)
  {
    return EPTMD_FAILED;
  }

  return EPTMD_SUCCESS;
}

static unsigned int ReadUserFile(int fd, void *p_buffer, unsigned int buffer_size)
{
  std::size_t total_size = 0;
  char *p_data = (char *)p_buffer;

  for(; buffer_size > total_size;)
  {
    std::size_t read_size = static_cast<std::size_t>(read(fd, p_data + total_size, (buffer_size - total_size)));

    if(0 == read_size)
    {
      break;
    }

    total_size += read_size;
  }

  return static_cast<unsigned int>(total_size);
}

static result_t WriteData(unsigned char *p_buffer, unsigned int size)
{
  char *p_data = (char *)p_buffer;
  result_t result = EPTMD_SUCCESS;
  unsigned int count;

  for(count = 0; size > count; count += result)
  {
    result = static_cast<result_t>(write(STDOUT_FILENO, (p_data + count), (size - count)));

    if(EPTMD_SUCCESS == result)
    {
      break;
    }
  }

  return (count == size) ? EPTMD_SUCCESS : EPTMD_FAILED;
}
