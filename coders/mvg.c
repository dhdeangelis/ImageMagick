/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            M   M  V   V   GGGG                              %
%                            MM MM  V   V  G                                  %
%                            M M M  V   V  G GG                               %
%                            M   M   V V   G   G                              %
%                            M   M    V     GGG                               %
%                                                                             %
%                                                                             %
%                 Read/Write Magick Vector Graphics Metafiles.                %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 April 2000                                  %
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
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMVGImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M V G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMVG() returns MagickTrue if the image format type, identified by the
%  magick string, is MVG.
%
%  The format of the IsMVG method is:
%
%      MagickBooleanType IsMVG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsMVG(const unsigned char *magick,const size_t length)
{
  if (length < 20)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"push graphic-context",20) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M V G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMVGImage creates a gradient image and initializes it to
%  the X server color range as specified by the filename.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadMVGImage method is:
%
%      Image *ReadMVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadMVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((image->columns == 0) || (image->rows == 0))
    {
      char
        primitive[MagickPathExtent];

      char
        *p;

      SegmentInfo
        bounds;

      /*
        Determine size of image canvas.
      */
      (void) memset(&bounds,0,sizeof(bounds));
      while (ReadBlobString(image,primitive) != (char *) NULL)
      {
        int
          count;

        for (p=primitive; (*p == ' ') || (*p == '\t'); p++) ;
        count=MagickSscanf(p,"viewbox %lf %lf %lf %lf",&bounds.x1,&bounds.y1,
          &bounds.x2,&bounds.y2);
        if (count != 4)
          continue;
        image->columns=CastDoubleToSizeT(floor((bounds.x2-bounds.x1)+0.5));
        image->rows=CastDoubleToSizeT(floor((bounds.y2-bounds.y1)+0.5));
        break;
      }
    }
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  if (draw_info->density != (char *) NULL)
    draw_info->density=DestroyString(draw_info->density);
  draw_info->affine.sx=image->resolution.x == 0.0 ? 1.0 : image->resolution.x/
    96.0;
  draw_info->affine.sy=image->resolution.y == 0.0 ? 1.0 : image->resolution.y/
    96.0;
  image->columns=CastDoubleToSizeT(draw_info->affine.sx*image->columns);
  image->rows=CastDoubleToSizeT(draw_info->affine.sy*image->rows);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  /*
    Render drawing.
  */
  if (GetBlobStreamData(image) == (unsigned char *) NULL)
    draw_info->primitive=FileToString(image->filename,~0UL,exception);
  else
    {
      MagickSizeType
        length;

      length=GetBlobSize(image);
      if (length == (MagickSizeType) ((size_t) length))
        {
          draw_info->primitive=(char *) AcquireQuantumMemory(1,(size_t) length+1);
          if (draw_info->primitive != (char *) NULL)
            {
              memcpy(draw_info->primitive,GetBlobStreamData(image),(size_t)
                length);
              draw_info->primitive[length]='\0';
            }
         }
     }
  if (draw_info->primitive == (char *) NULL)
    {
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (*draw_info->primitive == '@')
    {
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  (void) DrawImage(image,draw_info,exception);
  (void) SetImageArtifact(image,"mvg:vector-graphics",draw_info->primitive);
  draw_info=DestroyDrawInfo(draw_info);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M V G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMVGImage() adds properties for the MVG image format
%  to the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMVGImage method is:
%
%      size_t RegisterMVGImage(void)
%
*/
ModuleExport size_t RegisterMVGImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MVG","MVG","Magick Vector Graphics");
  entry->decoder=(DecodeImageHandler *) ReadMVGImage;
  entry->encoder=(EncodeImageHandler *) WriteMVGImage;
  entry->magick=(IsImageFormatHandler *) IsMVG;
  entry->format_type=ImplicitFormatType;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M V G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMVGImage() removes format registrations made by the
%  MVG module from the list of supported formats.
%
%  The format of the UnregisterMVGImage method is:
%
%      UnregisterMVGImage(void)
%
*/
ModuleExport void UnregisterMVGImage(void)
{
  (void) UnregisterMagickInfo("MVG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M V G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMVGImage() writes an image to a file in MVG image format.
%
%  The format of the WriteMVGImage method is:
%
%      MagickBooleanType WriteMVGImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteMVGImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  MagickBooleanType
    status;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  value=GetImageArtifact(image,"mvg:vector-graphics");
  if (value == (const char *) NULL)
    ThrowWriterException(OptionError,"NoImageVectorGraphics");
  status=OpenBlob(image_info,image,WriteBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) WriteBlob(image,strlen(value),(const unsigned char *) value);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
