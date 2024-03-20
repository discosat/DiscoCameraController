/*=============================================================================
  Copyright (C) 2014 - 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        PrintVmbVersion.h

  Description: Print Vmb version.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include "include/VmbCExamplesCommon/PrintVmbVersion.h"

#include "include/VmbCExamplesCommon/ErrorCodeToMessage.h"

#include <stdio.h>

#include <VmbC/VmbC.h>

void PrintVmbVersion()
{
    VmbVersionInfo_t    versionInfo;
    VmbError_t          result = VmbVersionQuery(&versionInfo, sizeof(versionInfo));
    if(VmbErrorSuccess == result)
    {
        printf("Vmb Version Major: %u Minor: %u Patch: %u\n\n", versionInfo.major, versionInfo.minor, versionInfo.patch);
    }
    else
    {
        printf("VmbVersionQuery failed with reason: %s\n\n", ErrorCodeToMessage(result));
    }
}


