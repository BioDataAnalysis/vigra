/************************************************************************/
/*                                                                      */
/*               Copyright 1998-2002 by Ullrich Koethe                  */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://hci.iwr.uni-heidelberg.de/vigra/                       */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/


#ifndef VIGRA_COPYIMAGE_HXX
#define VIGRA_COPYIMAGE_HXX

#include "utilities.hxx"
#include "multi_shape.hxx"

namespace vigra {

/** \addtogroup CopyAlgo Algorithms to Copy Images
    Copy images or regions
*/
//@{

/********************************************************/
/*                                                      */
/*                       copyLine                       */
/*                                                      */
/********************************************************/

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void
copyLine(SrcIterator s,
         SrcIterator send, SrcAccessor src,
         DestIterator d, DestAccessor dest)
{
    for(; s != send; ++s, ++d)
        dest.set(src(s), d);
}

template <class SrcIterator, class SrcAccessor,
          class MaskIterator, class MaskAccessor,
          class DestIterator, class DestAccessor>
void
copyLineIf(SrcIterator s,
           SrcIterator send, SrcAccessor src,
           MaskIterator m, MaskAccessor mask,
           DestIterator d, DestAccessor dest)
{
    for(; s != send; ++s, ++d, ++m)
        if(mask(m))
            dest.set(src(s), d);
}

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void
swapLine(SrcIterator s,
         SrcIterator send, SrcAccessor src,
         DestIterator d, DestAccessor dest)
{
    for(; s != send; ++s, ++d)
    {
        typename SrcAccessor::value_type t = src(s);
        src.set(dest(d), s);
        dest.set(t, d);
    }
}

/********************************************************/
/*                                                      */
/*                        copyImage                     */
/*                                                      */
/********************************************************/

/** \brief Copy source image into destination image.

    If necessary, type conversion takes place.
    Some variants of this function use accessors to access the pixel data.

    See \ref copyMultiArray() for a dimension-independent version of this algorithm.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        template <class T1, class S1,
                  class T2, class S2>
        void
        copyImage(MultiArrayView<2, T1, S1> const & src,
                  MultiArrayView<2, T2, S2> dest);
    }
    \endcode

    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
                  class DestImageIterator, class DestAccessor>
        void
        copyImage(SrcImageIterator src_upperleft, SrcImageIterator src_lowerright, SrcAccessor sa,
              DestImageIterator dest_upperleft, DestAccessor da)
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
                  class DestImageIterator, class DestAccessor>
        void
        copyImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                  pair<DestImageIterator, DestAccessor> dest)
    }
    \endcode

    <b> Usage:</b>

    <b>\#include</b> \<vigra/copyimage.hxx\><br>
    Namespace: vigra

    Use MultiArrayView API:
    \code
    MultiArray<2, int> src(Shape2(100, 200)),
                       dest(Shape2(100, 200));
    ...

    copyImage(src, dest);

    // equivalent to
    dest = src;
    \endcode

    Use iterator-based API with accessor:
    \code
    MultiArray<2, RGBValue<unsigned char> > src(Shape2(100, 200)),
    MultiArray<2, float>                    dest(Shape2(100, 200));

    // convert RGB to gray values in the fly
    copyImage(srcImageRange(src, RGBToGrayAccessor<RGBValue<unsigned char> >()),
              destImage(dest));
    \endcode

    <b> Required Interface:</b>

    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator      dest_upperleft;
    SrcImageIterator::row_iterator sx = src_upperleft.rowIterator();
    DestImageIterator::row_iterator dx = dest_upperleft.rowIterator();

    SrcAccessor src_accessor;
    DestAccessor dest_accessor;

    dest_accessor.set(src_accessor(sx), dx);
    \endcode
*/
doxygen_overloaded_function(template <...> void copyImage)

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
void
copyImage(SrcImageIterator src_upperleft,
          SrcImageIterator src_lowerright, SrcAccessor sa,
          DestImageIterator dest_upperleft, DestAccessor da)
{
    int w = src_lowerright.x - src_upperleft.x;

    for(; src_upperleft.y<src_lowerright.y; ++src_upperleft.y, ++dest_upperleft.y)
    {
        copyLine(src_upperleft.rowIterator(),
                 src_upperleft.rowIterator() + w, sa,
                 dest_upperleft.rowIterator(), da);
    }
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline
void
copyImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
          pair<DestImageIterator, DestAccessor> dest)
{
    copyImage(src.first, src.second, src.third,
              dest.first, dest.second);
}

template <class T1, class S1,
          class T2, class S2>
inline void
copyImage(MultiArrayView<2, T1, S1> const & src,
          MultiArrayView<2, T2, S2> dest)
{
    vigra_precondition(src.shape() == dest.shape(),
        "copyImage(): shape mismatch between input and output.");
    copyImage(srcImageRange(src), destImage(dest));
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
void
swapImageData(SrcImageIterator src_upperleft,
              SrcImageIterator src_lowerright, SrcAccessor sa,
              DestImageIterator dest_upperleft, DestAccessor da)
{
    int w = src_lowerright.x - src_upperleft.x;

    for(; src_upperleft.y<src_lowerright.y; ++src_upperleft.y, ++dest_upperleft.y)
    {
        swapLine(src_upperleft.rowIterator(),
                 src_upperleft.rowIterator() + w, sa,
                 dest_upperleft.rowIterator(), da);
    }
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline
void
swapImageData(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
              pair<DestImageIterator, DestAccessor> dest)
{
    swapImageData(src.first, src.second, src.third,
                  dest.first, dest.second);
}

template <class T1, class S1,
          class T2, class S2>
inline
void
swapImageData(MultiArrayView<2, T1, S1> const & src,
              MultiArrayView<2, T2, S2> dest)
{
    vigra_precondition(src.shape() == dest.shape(),
        "swapImageData(): shape mismatch between input and output.");
    swapImageData(srcImageRange(src), destImage(dest));
}

/********************************************************/
/*                                                      */
/*                       copyImageIf                    */
/*                                                      */
/********************************************************/

/** \brief Copy source ROI into destination image.

    Pixel values are copied whenever the return value of the mask's
    accessor is not zero.
    If necessary, type conversion takes place.
    Some variants of this function use accessors to access the pixel data.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        template <class T1, class S1,
                  class TM, class SM,
                  class T2, class S2>
        void
        copyImageIf(MultiArrayView<2, T1, S1> const & src,
                    MultiArrayView<2, TM, SM> const & mask,
                    MultiArrayView<2, T2, S2> dest);
    }
    \endcode

    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
              class MaskImageIterator, class MaskAccessor,
              class DestImageIterator, clas DestAccessor>
        void
        copyImageIf(SrcImageIterator src_upperleft,
            SrcImageIterator src_lowerright, SrcAccessor sa,
            MaskImageIterator mask_upperleft, MaskAccessor ma,
            DestImageIterator dest_upperleft, DestAccessor da)
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
              class MaskImageIterator, class MaskAccessor,
              class DestImageIterator, clas DestAccessor>
        void
        copyImageIf(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
            pair<MaskImageIterator, MaskAccessor> mask,
            pair<DestImageIterator, DestAccessor> dest)
    }
    \endcode

    <b> Usage:</b>

    <b>\#include</b> \<vigra/copyimage.hxx\><br>
    Namespace: vigra

    Use MultiArrayView API:
    \code
    MultiArray<2, int> src(Shape2(100, 200)),
                       mask(Shape2(100, 200)),
                       dest(Shape2(100, 200));
    ...

    copyImageIf(src, mask, dest);
    \endcode

    Use iterator-based API with accessor:
    \code
    MultiArray<2, RGBValue<unsigned char> > src(Shape2(100, 200)),
    MultiArray<2, unsigned char>            mask(Shape2(100, 200));
    MultiArray<2, float>                    dest(Shape2(100, 200));

    // convert RGB to gray values in the fly
    copyImageIf(srcImageRange(src, RGBToGrayAccessor<RGBValue<unsigned char> >()),
                maskImage(mask), destImage(dest));
    \endcode

    <b> Required Interface:</b>

    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator dest_upperleft;
    MaskImageIterator mask_upperleft;
    SrcImageIterator::row_iterator sx = src_upperleft.rowIterator();
    MaskImageIterator::row_iterator mx = mask_upperleft.rowIterator();
    DestImageIterator::row_iterator dx = dest_upperleft.rowIterator();

    SrcAccessor src_accessor;
    DestAccessor dest_accessor;
    MaskAccessor mask_accessor;
    Functor functor;

    if(mask_accessor(mx))
        dest_accessor.set(src_accessor(sx), dx);

    \endcode
*/
doxygen_overloaded_function(template <...> void copyImageIf)

template <class SrcImageIterator, class SrcAccessor,
          class MaskImageIterator, class MaskAccessor,
          class DestImageIterator, class DestAccessor>
void
copyImageIf(SrcImageIterator src_upperleft,
            SrcImageIterator src_lowerright, SrcAccessor sa,
            MaskImageIterator mask_upperleft, MaskAccessor ma,
        DestImageIterator dest_upperleft, DestAccessor da)
{
    int w = src_lowerright.x - src_upperleft.x;

    for(; src_upperleft.y<src_lowerright.y;
             ++src_upperleft.y, ++mask_upperleft.y, ++dest_upperleft.y)
    {
        copyLineIf(src_upperleft.rowIterator(),
                   src_upperleft.rowIterator() + w, sa,
                   mask_upperleft.rowIterator(), ma,
                   dest_upperleft.rowIterator(), da);
    }
}

template <class SrcImageIterator, class SrcAccessor,
          class MaskImageIterator, class MaskAccessor,
          class DestImageIterator, class DestAccessor>
inline
void
copyImageIf(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
            pair<MaskImageIterator, MaskAccessor> mask,
            pair<DestImageIterator, DestAccessor> dest)
{
    copyImageIf(src.first, src.second, src.third,
                mask.first, mask.second,
                dest.first, dest.second);
}

template <class T1, class S1,
          class TM, class SM,
          class T2, class S2>
inline void
copyImageIf(MultiArrayView<2, T1, S1> const & src,
            MultiArrayView<2, TM, SM> const & mask,
            MultiArrayView<2, T2, S2> dest)
{
    vigra_precondition(src.shape() == mask.shape() && src.shape() == dest.shape(),
        "copyImageIf(): shape mismatch between input and output.");
    copyImageIf(srcImageRange(src),
                maskImage(mask),
                destImage(dest));
}

//@}

} // namespace vigra

#endif // VIGRA_COPYIMAGE_HXX
