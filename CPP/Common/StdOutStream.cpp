// Common/StdOutStream.cpp

#include "StdAfx.h"

#include <tchar.h>

#include "IntToString.h"
#include "StdOutStream.h"
#include "StringConvert.h"
#include "UTFConvert.h"

static const char kNewLineChar =  '\n';

static const char *kFileOpenMode = "wt";

extern int g_CodePage;

CStdOutStream g_StdOut(stdout);
CStdOutStream g_StdErr(stderr);

bool CStdOutStream::Open(const char *fileName) throw()
{
  Close();
  _stream = fopen(fileName, kFileOpenMode);
  _streamIsOpen = (_stream != 0);
  return _streamIsOpen;
}

bool CStdOutStream::Close() throw()
{
  if (!_streamIsOpen)
    return true;
  if (fclose(_stream) != 0)
    return false;
  _stream = 0;
  _streamIsOpen = false;
  return true;
}

bool CStdOutStream::Flush() throw()
{
  return (fflush(_stream) == 0);
}

CStdOutStream & endl(CStdOutStream & outStream) throw()
{
  return outStream << kNewLineChar;
}

CStdOutStream & CStdOutStream::operator<<(const wchar_t *s)
{
  int codePage = g_CodePage;
  if (codePage == -1)
    codePage = CP_OEMCP;
  AString dest;
  if (codePage == CP_UTF8)
    ConvertUnicodeToUTF8(s, dest);
  else
    UnicodeStringToMultiByte2(dest, s, (UINT)codePage);
  return operator<<((const char *)dest);
}

void StdOut_Convert_UString_to_AString(const UString &s, AString &temp)
{
  int codePage = g_CodePage;
  if (codePage == -1)
    codePage = CP_OEMCP;
  if (codePage == CP_UTF8)
    ConvertUnicodeToUTF8(s, temp);
  else
    UnicodeStringToMultiByte2(temp, s, (UINT)codePage);
}

void CStdOutStream::PrintUString(const UString &s, AString &temp)
{
  StdOut_Convert_UString_to_AString(s, temp);
  *this << (const char *)temp;
}


static const wchar_t kReplaceChar = '_';

void CStdOutStream::Normalize_UString__LF_Allowed(UString &s)
{
  unsigned len = s.Len();
  wchar_t *d = s.GetBuf();

  if (IsTerminalMode)
    for (unsigned i = 0; i < len; i++)
    {
      wchar_t c = d[i];
      if (c <= 13 && c >= 7 && c != '\n')
        d[i] = kReplaceChar;
    }
}

void CStdOutStream::Normalize_UString(UString &s)
{
  unsigned len = s.Len();
  wchar_t *d = s.GetBuf();

  if (IsTerminalMode)
    for (unsigned i = 0; i < len; i++)
    {
      wchar_t c = d[i];
      if (c <= 13 && c >= 7)
        d[i] = kReplaceChar;
    }
  else
    for (unsigned i = 0; i < len; i++)
    {
      wchar_t c = d[i];
      if (c == '\n')
        d[i] = kReplaceChar;
    }
}

void CStdOutStream::NormalizePrint_UString(const UString &s, UString &tempU, AString &tempA)
{
  tempU = s;
  Normalize_UString(tempU);
  PrintUString(tempU, tempA);
}

void CStdOutStream::NormalizePrint_UString(const UString &s)
{
  NormalizePrint_wstr(s);
}

void CStdOutStream::NormalizePrint_wstr(const wchar_t *s)
{
  UString tempU = s;
  Normalize_UString(tempU);
  AString tempA;
  PrintUString(tempU, tempA);
}


CStdOutStream & CStdOutStream::operator<<(Int32 number) throw()
{
  char s[32];
  ConvertInt64ToString(number, s);
  return operator<<(s);
}

CStdOutStream & CStdOutStream::operator<<(Int64 number) throw()
{
  char s[32];
  ConvertInt64ToString(number, s);
  return operator<<(s);
}

CStdOutStream & CStdOutStream::operator<<(UInt32 number) throw()
{
  char s[16];
  ConvertUInt32ToString(number, s);
  return operator<<(s);
}

CStdOutStream & CStdOutStream::operator<<(UInt64 number) throw()
{
  char s[32];
  ConvertUInt64ToString(number, s);
  return operator<<(s);
}
