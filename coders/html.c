/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        H   H  TTTTT  M   M  L                               %
%                        H   H    T    MM MM  L                               %
%                        HHHHH    T    M M M  L                               %
%                        H   H    T    M   M  L                               %
%                        H   H    T    M   M  LLLLL                           %
%                                                                             %
%                                                                             %
%                  Write A Client-Side Image Map Using                        %
%                 Image Montage & Directory Information.                      %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/paint.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHTMLImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H T M L                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHTML() returns MagickTrue if the image format type, identified by the
%  magick string, is HTML.
%
%  The format of the IsHTML method is:
%
%      MagickBooleanType IsHTML(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsHTML(const unsigned char *magick,const size_t length)
{
  if (length < 6)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick+1,"html",5) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H T M L I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHTMLImage() adds properties for the HTML image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterHTMLImage method is:
%
%      size_t RegisterHTMLImage(void)
%
*/
ModuleExport size_t RegisterHTMLImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("HTML","HTM",
    "Hypertext Markup Language and a client-side image map");
  entry->encoder=(EncodeImageHandler *) WriteHTMLImage;
  entry->magick=(IsImageFormatHandler *) IsHTML;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("HTML","HTML",
    "Hypertext Markup Language and a client-side image map");
  entry->encoder=(EncodeImageHandler *) WriteHTMLImage;
  entry->magick=(IsImageFormatHandler *) IsHTML;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("HTML","SHTML",
    "Hypertext Markup Language and a client-side image map");
  entry->encoder=(EncodeImageHandler *) WriteHTMLImage;
  entry->magick=(IsImageFormatHandler *) IsHTML;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H T M L I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHTMLImage() removes format registrations made by the
%  HTML module from the list of supported formats.
%
%  The format of the UnregisterHTMLImage method is:
%
%      UnregisterHTMLImage(void)
%
*/
ModuleExport void UnregisterHTMLImage(void)
{
  (void) UnregisterMagickInfo("HTM");
  (void) UnregisterMagickInfo("HTML");
  (void) UnregisterMagickInfo("SHTML");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H T M L I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteHTMLImage() writes an image in the HTML encoded image format.
%
%  The format of the WriteHTMLImage method is:
%
%      MagickBooleanType WriteHTMLImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static ssize_t WriteURLComponent(Image *image,const int c)
{
  char
    encoding[MagickPathExtent],
    html5;
  
  html5=isalnum(c) != 0 || (c == '-') || (c == '_') || (c == '.') ||
    (c == '!') || (c == '~') || (c == '*') || (c == '\'') || (c == '(') ||
    (c == ')') ?  (char) c : 0;
  if (html5 != 0)
    (void) FormatLocaleString(encoding,MagickPathExtent,"%c",html5);
  else
    (void) FormatLocaleString(encoding,MagickPathExtent,"%%%02X",c);
  return(WriteBlobString(image,encoding));
}

static MagickBooleanType WriteHTMLImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    basename[MagickPathExtent],
    buffer[MagickPathExtent],
    filename[MagickPathExtent],
    mapname[MagickPathExtent],
    url[MagickPathExtent];

  Image
    *next;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  char
    *p;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) CloseBlob(image);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  *url='\0';
  if ((LocaleCompare(image_info->magick,"FTP") == 0) ||
      (LocaleCompare(image_info->magick,"HTTP") == 0))
    {
      /*
        Extract URL base from filename.
      */
      p=strrchr(image->filename,'/');
      if (p != (char *) NULL)
        {
          p++;
          (void) CopyMagickString(url,image_info->magick,MagickPathExtent);
          (void) ConcatenateMagickString(url,":",MagickPathExtent);
          url[strlen(url)+p-image->filename]='\0';
          (void) ConcatenateMagickString(url,image->filename,(size_t)
            (p-image->filename+2));
          (void) CopyMagickString(image->filename,p,MagickPathExtent);
        }
    }
  /*
    Refer to image map file.
  */
  (void) CopyMagickString(filename,image->filename,MagickPathExtent);
  AppendImageFormat("map",filename);
  GetPathComponent(filename,BasePath,basename);
  (void) CopyMagickString(mapname,basename,MagickPathExtent);
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  (void) CopyMagickString(filename,image->filename,MagickPathExtent);
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  write_info->adjoin=MagickTrue;
  status=MagickTrue;
  if (LocaleCompare(image_info->magick,"SHTML") != 0)
    {
      const char
        *value;

      /*
        Open output image file.
      */
      assert(exception != (ExceptionInfo *) NULL);
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      /*
        Write the HTML image file.
      */
      (void) WriteBlobString(image,"<?xml version=\"1.0\" "
        "encoding=\"UTF-8\"?>\n");
      (void) WriteBlobString(image,"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML "
        "1.0 Strict//EN\" "
        "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
      (void) WriteBlobString(image,"<html>\n");
      (void) WriteBlobString(image,"<head>\n");
      value=GetImageProperty(image,"label",exception);
      if (value != (const char *) NULL)
        (void) FormatLocaleString(buffer,MagickPathExtent,"<title>%s</title>\n",
          value);
      else
        {
          GetPathComponent(filename,BasePath,basename);
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "<title>%s</title>\n",basename);
        }
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"</head>\n");
      (void) WriteBlobString(image,"<body style=\"text-align: center;\">\n");
      (void) FormatLocaleString(buffer,MagickPathExtent,"<h1>%s</h1>\n",
        image->filename);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"<div>\n");
      (void) CopyMagickString(filename,image->filename,MagickPathExtent);
      AppendImageFormat("png",filename);
      (void) FormatLocaleString(buffer,MagickPathExtent,"<img usemap=\"#%s\" "
        "src=\"%s\" style=\"border: 0;\" alt=\"Image map\" />\n",mapname,
        filename);
      (void) WriteBlobString(image,buffer);
      /*
        Determine the size and location of each image tile.
      */
      SetGeometry(image,&geometry);
      if (image->montage != (char *) NULL)
        (void) ParseAbsoluteGeometry(image->montage,&geometry);
      /*
        Write an image map.
      */
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "<map id=\"%s\" name=\"%s\">\n",mapname,mapname);
      (void) WriteBlobString(image,buffer);
      (void) FormatLocaleString(buffer,MagickPathExtent,"  <area href=\"%s",
        url);
      (void) WriteBlobString(image,buffer);
      if (image->directory == (char *) NULL)
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "%s\" shape=\"rect\" coords=\"0,0,%.20g,%.20g\" alt=\"\" />\n",
            image->filename,(double) geometry.width-1,(double) geometry.height-
            1);
          (void) WriteBlobString(image,buffer);
        }
      else
        for (p=image->directory; *p != '\0'; p++)
          if (*p != '\xff')
            (void) WriteURLComponent(image,(unsigned char) *p);
          else
            {
              (void) FormatLocaleString(buffer,MagickPathExtent,"\" shape="
                "\"rect\" coords=\"%.20g,%.20g,%.20g,%.20g\" alt=\"\" />\n",
                (double) geometry.x,(double) geometry.y,(double) (geometry.x+
                (ssize_t) geometry.width-1),(double) (geometry.y+(ssize_t)
                geometry.height-1));
              (void) WriteBlobString(image,buffer);
              if (*(p+1) != '\0')
                {
                  (void) FormatLocaleString(buffer,MagickPathExtent,
                    "  <area href=%s\"",url);
                  (void) WriteBlobString(image,buffer);
                }
              geometry.x+=(ssize_t) geometry.width;
              if ((geometry.x+4) >= (ssize_t) image->columns)
                {
                  geometry.x=0;
                  geometry.y+=(ssize_t) geometry.height;
                }
            }
      (void) WriteBlobString(image,"</map>\n");
      (void) CopyMagickString(filename,image->filename,MagickPathExtent);
      (void) WriteBlobString(image,"</div>\n");
      (void) WriteBlobString(image,"</body>\n");
      (void) WriteBlobString(image,"</html>\n");
      (void) CloseBlob(image);
      /*
        Write the image as PNG.
      */
      (void) CopyMagickString(image->filename,filename,MagickPathExtent);
      AppendImageFormat("png",image->filename);
      next=GetNextImageInList(image);
      image->next=NewImageList();
      (void) CopyMagickString(image->magick,"PNG",MagickPathExtent);
      (void) WriteImage(write_info,image,exception);
      image->next=next;
      /*
        Determine image map filename.
      */
      GetPathComponent(image->filename,BasePath,filename);
      (void) ConcatenateMagickString(filename,"_map.shtml",MagickPathExtent);
      (void) CopyMagickString(image->filename,filename,MagickPathExtent);
    }
  /*
    Open image map.
  */
  status=OpenBlob(write_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  write_info=DestroyImageInfo(write_info);
  /*
    Determine the size and location of each image tile.
  */
  SetGeometry(image,&geometry);
  if (image->montage != (char *) NULL)
    (void) ParseAbsoluteGeometry(image->montage,&geometry);
  /*
    Write an image map.
  */
  (void) FormatLocaleString(buffer,MagickPathExtent,
    "<map id=\"%s\" name=\"%s\">\n",mapname,mapname);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MagickPathExtent,"  <area href=\"%s",url);
  (void) WriteBlobString(image,buffer);
  if (image->directory == (char *) NULL)
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "%s\" shape=\"rect\" coords=\"0,0,%.20g,%.20g\" alt=\"\" />\n",
        image->filename,(double) geometry.width-1,(double) geometry.height-1);
      (void) WriteBlobString(image,buffer);
    }
  else
    for (p=image->directory; *p != '\0'; p++)
      if (*p != '\xff')
        (void) WriteURLComponent(image,(unsigned char) *p);
      else
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"\" shape=\"rect\""
            " coords=\"%.20g,%.20g,%.20g,%.20g\" alt=\"\" />\n",
            (double) geometry.x,(double) geometry.y,geometry.x+(double)
            geometry.width-1,geometry.y+(double) geometry.height-1);
          (void) WriteBlobString(image,buffer);
          if (*(p+1) != '\0')
            {
              (void) FormatLocaleString(buffer,MagickPathExtent,
                "  <area href=%s\"",url);
              (void) WriteBlobString(image,buffer);
            }
          geometry.x+=(ssize_t) geometry.width;
          if ((geometry.x+4) >= (ssize_t) image->columns)
            {
              geometry.x=0;
              geometry.y+=(ssize_t) geometry.height;
            }
        }
  (void) WriteBlobString(image,"</map>\n");
  (void) CloseBlob(image);
  (void) CopyMagickString(image->filename,filename,MagickPathExtent);
  return(status);
}
