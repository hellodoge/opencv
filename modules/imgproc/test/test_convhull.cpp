/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "test_precomp.hpp"

namespace opencv_test { namespace {

/*static int
cvTsPointConvexPolygon( CvPoint2D32f pt, CvPoint2D32f* v, int n )
{
    CvPoint2D32f v0 = v[n-1];
    int i, sign = 0;

    for( i = 0; i < n; i++ )
    {
        CvPoint2D32f v1 = v[i];
        float dx = pt.x - v0.x, dy = pt.y - v0.y;
        float dx1 = v1.x - v0.x, dy1 = v1.y - v0.y;
        double t = (double)dx*dy1 - (double)dx1*dy;
        if( fabs(t) > DBL_EPSILON )
        {
            if( t*sign < 0 )
                break;
            if( sign == 0 )
                sign = t < 0 ? -1 : 1;
        }
        else if( fabs(dx) + fabs(dy) < DBL_EPSILON )
            return i+1;
        v0 = v1;
    }

    return i < n ? -1 : 0;
}*/

CV_INLINE double
cvTsDist( CvPoint2D32f a, CvPoint2D32f b )
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}
CV_INLINE double
cvTsDist( const Point2f& a, const Point2f& b )
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

CV_INLINE double
cvTsPtLineDist( CvPoint2D32f pt, CvPoint2D32f a, CvPoint2D32f b )
{
    double d0 = cvTsDist( pt, a ), d1;
    double dd = cvTsDist( a, b );
    if( dd < FLT_EPSILON )
        return d0;
    d1 = cvTsDist( pt, b );
    dd = fabs((double)(pt.x - a.x)*(b.y - a.y) - (double)(pt.y - a.y)*(b.x - a.x))/dd;
    d0 = MIN( d0, d1 );
    return MIN( d0, dd );
}

static double
cvTsPointPolygonTest( CvPoint2D32f pt, const CvPoint2D32f* vv, int n, int* _idx=0, int* _on_edge=0 )
{
    int i;
    Point2f v = vv[n-1], v0;
    double min_dist_num = FLT_MAX, min_dist_denom = 1;
    int min_dist_idx = -1, min_on_edge = 0;
    int counter = 0;
    double result;

    for( i = 0; i < n; i++ )
    {
        double dx, dy, dx1, dy1, dx2, dy2, dist_num, dist_denom = 1;
        int on_edge = 0, idx = i;

        v0 = v; v = vv[i];
        dx = v.x - v0.x; dy = v.y - v0.y;
        dx1 = pt.x - v0.x; dy1 = pt.y - v0.y;
        dx2 = pt.x - v.x; dy2 = pt.y - v.y;

        if( dx2*dx + dy2*dy >= 0 )
            dist_num = dx2*dx2 + dy2*dy2;
        else if( dx1*dx + dy1*dy <= 0 )
        {
            dist_num = dx1*dx1 + dy1*dy1;
            idx = i - 1;
            if( idx < 0 ) idx = n-1;
        }
        else
        {
            dist_num = (dy1*dx - dx1*dy);
            dist_num *= dist_num;
            dist_denom = dx*dx + dy*dy;
            on_edge = 1;
        }

        if( dist_num*min_dist_denom < min_dist_num*dist_denom )
        {
            min_dist_num = dist_num;
            min_dist_denom = dist_denom;
            min_dist_idx = idx;
            min_on_edge = on_edge;
            if( min_dist_num == 0 )
                break;
        }

        if( (v0.y <= pt.y && v.y <= pt.y) ||
            (v0.y > pt.y && v.y > pt.y) ||
            (v0.x < pt.x && v.x < pt.x) )
            continue;

        dist_num = dy1*dx - dx1*dy;
        if( dy < 0 )
            dist_num = -dist_num;
        counter += dist_num > 0;
    }

    result = sqrt(min_dist_num/min_dist_denom);
    if( counter % 2 == 0 )
        result = -result;

    if( _idx )
        *_idx = min_dist_idx;
    if( _on_edge )
        *_on_edge = min_on_edge;

    return result;
}

static cv::Point2f
cvTsMiddlePoint(const cv::Point2f &a, const cv::Point2f &b)
{
    return cv::Point2f((a.x + b.x) / 2, (a.y + b.y) / 2);
}

static bool
cvTsIsPointOnLineSegment(const cv::Point2f &x, const cv::Point2f &a, const cv::Point2f &b)
{
    double d1 = cvTsDist(cvPoint2D32f(x.x, x.y), cvPoint2D32f(a.x, a.y));
    double d2 = cvTsDist(cvPoint2D32f(x.x, x.y), cvPoint2D32f(b.x, b.y));
    double d3 = cvTsDist(cvPoint2D32f(a.x, a.y), cvPoint2D32f(b.x, b.y));

    return (abs(d1 + d2 - d3) <= (1E-5));
}


/****************************************************************************************\
*                              Base class for shape descriptor tests                     *
\****************************************************************************************/

class CV_BaseShapeDescrTest : public cvtest::BaseTest
{
public:
    CV_BaseShapeDescrTest();
    virtual ~CV_BaseShapeDescrTest();
    void clear();

protected:
    int read_params( CvFileStorage* fs );
    void run_func(void);
    int prepare_test_case( int test_case_idx );
    int validate_test_results( int test_case_idx );
    virtual void generate_point_set( void* points );
    virtual void extract_points();

    int min_log_size;
    int max_log_size;
    int dims;
    bool enable_flt_points;

    CvMemStorage* storage;
    CvSeq* points1;
    CvMat* points2;
    void* points;
    void* result;
    double low_high_range;
    Scalar low, high;

    bool test_cpp;
};


CV_BaseShapeDescrTest::CV_BaseShapeDescrTest()
{
    points1 = 0;
    points2 = 0;
    points = 0;
    storage = 0;
    test_case_count = 500;
    min_log_size = 0;
    max_log_size = 10;
    low = high = cvScalarAll(0);
    low_high_range = 50;
    dims = 2;
    enable_flt_points = true;

    test_cpp = false;
}


CV_BaseShapeDescrTest::~CV_BaseShapeDescrTest()
{
    clear();
}


void CV_BaseShapeDescrTest::clear()
{
    cvtest::BaseTest::clear();
    cvReleaseMemStorage( &storage );
    cvReleaseMat( &points2 );
    points1 = 0;
    points = 0;
}


int CV_BaseShapeDescrTest::read_params( CvFileStorage* fs )
{
    int code = cvtest::BaseTest::read_params( fs );
    if( code < 0 )
        return code;

    test_case_count = cvReadInt( find_param( fs, "struct_count" ), test_case_count );
    min_log_size = cvReadInt( find_param( fs, "min_log_size" ), min_log_size );
    max_log_size = cvReadInt( find_param( fs, "max_log_size" ), max_log_size );

    min_log_size = cvtest::clipInt( min_log_size, 0, 8 );
    max_log_size = cvtest::clipInt( max_log_size, 0, 10 );
    if( min_log_size > max_log_size )
    {
        int t;
        CV_SWAP( min_log_size, max_log_size, t );
    }

    return 0;
}


void CV_BaseShapeDescrTest::generate_point_set( void* pointsSet )
{
    RNG& rng = ts->get_rng();
    int i, k, n, total, point_type;
    CvSeqReader reader;
    uchar* data = 0;
    double a[4], b[4];

    for( k = 0; k < 4; k++ )
    {
        a[k] = high.val[k] - low.val[k];
        b[k] = low.val[k];
    }
    memset( &reader, 0, sizeof(reader) );

    if( CV_IS_SEQ(pointsSet) )
    {
        CvSeq* ptseq = (CvSeq*)pointsSet;
        total = ptseq->total;
        point_type = CV_SEQ_ELTYPE(ptseq);
        cvStartReadSeq( ptseq, &reader );
    }
    else
    {
        CvMat* ptm = (CvMat*)pointsSet;
        CV_Assert( CV_IS_MAT(ptm) && CV_IS_MAT_CONT(ptm->type) );
        total = ptm->rows + ptm->cols - 1;
        point_type = CV_MAT_TYPE(ptm->type);
        data = ptm->data.ptr;
    }

    n = CV_MAT_CN(point_type);
    point_type = CV_MAT_DEPTH(point_type);

    CV_Assert( (point_type == CV_32S || point_type == CV_32F) && n <= 4 );

    for( i = 0; i < total; i++ )
    {
        int* pi;
        float* pf;
        if( reader.ptr )
        {
            pi = (int*)reader.ptr;
            pf = (float*)reader.ptr;
            CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
        }
        else
        {
            pi = (int*)data + i*n;
            pf = (float*)data + i*n;
        }
        if( point_type == CV_32S )
            for( k = 0; k < n; k++ )
                pi[k] = cvRound(cvtest::randReal(rng)*a[k] + b[k]);
        else
            for( k = 0; k < n; k++ )
                pf[k] = (float)(cvtest::randReal(rng)*a[k] + b[k]);
    }
}


int CV_BaseShapeDescrTest::prepare_test_case( int test_case_idx )
{
    int size;
    int use_storage = 0;
    int point_type;
    int i;
    RNG& rng = ts->get_rng();

    cvtest::BaseTest::prepare_test_case( test_case_idx );

    clear();
    size = cvRound( exp((cvtest::randReal(rng) * (max_log_size - min_log_size) + min_log_size)*CV_LOG2) );
    use_storage = cvtest::randInt(rng) % 2;
    point_type = CV_MAKETYPE(cvtest::randInt(rng) %
        (enable_flt_points ? 2 : 1) ? CV_32F : CV_32S, dims);

    if( use_storage )
    {
        storage = cvCreateMemStorage( (cvtest::randInt(rng)%10 + 1)*1024 );
        points1 = cvCreateSeq( point_type, sizeof(CvSeq), CV_ELEM_SIZE(point_type), storage );
        cvSeqPushMulti( points1, 0, size );
        points = points1;
    }
    else
    {
        int rows = 1, cols = size;
        if( cvtest::randInt(rng) % 2 )
            rows = size, cols = 1;

        points2 = cvCreateMat( rows, cols, point_type );
        points = points2;
    }

    for( i = 0; i < 4; i++ )
    {
        low.val[i] = (cvtest::randReal(rng)-0.5)*low_high_range*2;
        high.val[i] = (cvtest::randReal(rng)-0.5)*low_high_range*2;
        if( low.val[i] > high.val[i] )
        {
            double t;
            CV_SWAP( low.val[i], high.val[i], t );
        }
        if( high.val[i] < low.val[i] + 1 )
            high.val[i] += 1;
    }

    generate_point_set( points );

    test_cpp = (cvtest::randInt(rng) & 16) == 0;
    return 1;
}


void CV_BaseShapeDescrTest::extract_points()
{
    if( points1 )
    {
        points2 = cvCreateMat( 1, points1->total, CV_SEQ_ELTYPE(points1) );
        cvCvtSeqToArray( points1, points2->data.ptr );
    }

    if( CV_MAT_DEPTH(points2->type) != CV_32F && enable_flt_points )
    {
        CvMat tmp = cvMat( points2->rows, points2->cols,
            (points2->type & ~CV_MAT_DEPTH_MASK) | CV_32F, points2->data.ptr );
        cvConvert( points2, &tmp );
    }
}


void CV_BaseShapeDescrTest::run_func(void)
{
}


int CV_BaseShapeDescrTest::validate_test_results( int /*test_case_idx*/ )
{
    extract_points();
    return 0;
}


/****************************************************************************************\
*                                     Convex Hull Test                                   *
\****************************************************************************************/

class CV_ConvHullTest : public CV_BaseShapeDescrTest
{
public:
    CV_ConvHullTest();
    virtual ~CV_ConvHullTest();
    void clear();

protected:
    void run_func(void);
    int prepare_test_case( int test_case_idx );
    int validate_test_results( int test_case_idx );

    CvSeq* hull1;
    CvMat* hull2;
    void* hull_storage;
    int orientation;
    int return_points;
};


CV_ConvHullTest::CV_ConvHullTest()
{
    hull1 = 0;
    hull2 = 0;
    hull_storage = 0;
    orientation = return_points = 0;
}


CV_ConvHullTest::~CV_ConvHullTest()
{
    clear();
}


void CV_ConvHullTest::clear()
{
    CV_BaseShapeDescrTest::clear();
    cvReleaseMat( &hull2 );
    hull1 = 0;
    hull_storage = 0;
}


int CV_ConvHullTest::prepare_test_case( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::prepare_test_case( test_case_idx );
    int use_storage_for_hull = 0;
    RNG& rng = ts->get_rng();

    if( code <= 0 )
        return code;

    orientation = cvtest::randInt(rng) % 2 ? CV_CLOCKWISE : CV_COUNTER_CLOCKWISE;
    return_points = cvtest::randInt(rng) % 2;

    use_storage_for_hull = (cvtest::randInt(rng) % 2) && !test_cpp;
    if( use_storage_for_hull )
    {
        if( !storage )
            storage = cvCreateMemStorage( (cvtest::randInt(rng)%10 + 1)*1024 );
        hull_storage = storage;
    }
    else
    {
        int rows, cols;
        int sz = points1 ? points1->total : points2->cols + points2->rows - 1;
        int point_type = points1 ? CV_SEQ_ELTYPE(points1) : CV_MAT_TYPE(points2->type);

        if( cvtest::randInt(rng) % 2 )
            rows = sz, cols = 1;
        else
            rows = 1, cols = sz;

        hull2 = cvCreateMat( rows, cols, return_points ? point_type : CV_32SC1 );
        hull_storage = hull2;
    }

    return code;
}


void CV_ConvHullTest::run_func()
{
    if(!test_cpp)
        hull1 = cvConvexHull2( points, hull_storage, orientation, return_points );
    else
    {
        cv::Mat _points = cv::cvarrToMat(points);
        bool clockwise = orientation == CV_CLOCKWISE;
        size_t n = 0;
        if( !return_points )
        {
            std::vector<int> _hull;
            cv::convexHull(_points, _hull, clockwise);
            n = _hull.size();
            memcpy(hull2->data.ptr, &_hull[0], n*sizeof(_hull[0]));
        }
        else if(_points.type() == CV_32SC2)
        {
            std::vector<cv::Point> _hull;
            cv::convexHull(_points, _hull, clockwise);
            n = _hull.size();
            memcpy(hull2->data.ptr, &_hull[0], n*sizeof(_hull[0]));
        }
        else if(_points.type() == CV_32FC2)
        {
            std::vector<cv::Point2f> _hull;
            cv::convexHull(_points, _hull, clockwise);
            n = _hull.size();
            memcpy(hull2->data.ptr, &_hull[0], n*sizeof(_hull[0]));
        }
        if(hull2->rows > hull2->cols)
            hull2->rows = (int)n;
        else
            hull2->cols = (int)n;
    }
}


int CV_ConvHullTest::validate_test_results( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    CvMat* hull = 0;
    CvMat* mask = 0;
    int i, point_count, hull_count;
    CvPoint2D32f *p, *h;
    CvSeq header, hheader, *ptseq, *hseq;
    CvSeqBlock block, hblock;

    if( points1 )
        ptseq = points1;
    else
        ptseq = cvMakeSeqHeaderForArray( CV_MAT_TYPE(points2->type),
            sizeof(CvSeq), CV_ELEM_SIZE(points2->type), points2->data.ptr,
            points2->rows + points2->cols - 1, &header, &block );
    point_count = ptseq->total;
    p = (CvPoint2D32f*)(points2->data.ptr);

    if( hull1 )
        hseq = hull1;
    else
        hseq = cvMakeSeqHeaderForArray( CV_MAT_TYPE(hull2->type),
            sizeof(CvSeq), CV_ELEM_SIZE(hull2->type), hull2->data.ptr,
            hull2->rows + hull2->cols - 1, &hheader, &hblock );
    hull_count = hseq->total;
    hull = cvCreateMat( 1, hull_count, CV_32FC2 );
    mask = cvCreateMat( 1, hull_count, CV_8UC1 );
    cvZero( mask );
    Mat _mask = cvarrToMat(mask);

    h = (CvPoint2D32f*)(hull->data.ptr);

    // extract convex hull points
    if( return_points )
    {
        cvCvtSeqToArray( hseq, hull->data.ptr );
        if( CV_SEQ_ELTYPE(hseq) != CV_32FC2 )
        {
            CvMat tmp = cvMat( hull->rows, hull->cols, CV_32SC2, hull->data.ptr );
            cvConvert( &tmp, hull );
        }
    }
    else
    {
        CvSeqReader reader;
        cvStartReadSeq( hseq, &reader );

        for( i = 0; i < hull_count; i++ )
        {
            schar* ptr = reader.ptr;
            int idx;
            CV_NEXT_SEQ_ELEM( hseq->elem_size, reader );

            if( hull1 )
                idx = cvSeqElemIdx( ptseq, *(uchar**)ptr );
            else
                idx = *(int*)ptr;

            if( idx < 0 || idx >= point_count )
            {
                ts->printf( cvtest::TS::LOG, "Invalid convex hull point #%d\n", i );
                code = cvtest::TS::FAIL_INVALID_OUTPUT;
                goto _exit_;
            }
            h[i] = p[idx];
        }
    }

    // check that the convex hull is a convex polygon
    if( hull_count >= 3 )
    {
        CvPoint2D32f pt0 = h[hull_count-1];
        for( i = 0; i < hull_count; i++ )
        {
            int j = i+1;
            CvPoint2D32f pt1 = h[i], pt2 = h[j < hull_count ? j : 0];
            float dx0 = pt1.x - pt0.x, dy0 = pt1.y - pt0.y;
            float dx1 = pt2.x - pt1.x, dy1 = pt2.y - pt1.y;
            double t = (double)dx0*dy1 - (double)dx1*dy0;
            if( (t < 0) ^ (orientation != CV_COUNTER_CLOCKWISE) )
            {
                ts->printf( cvtest::TS::LOG, "The convex hull is not convex or has a wrong orientation (vtx %d)\n", i );
                code = cvtest::TS::FAIL_INVALID_OUTPUT;
                goto _exit_;
            }
            pt0 = pt1;
        }
    }

    // check that all the points are inside the hull or on the hull edge
    // and at least hull_point points are at the hull vertices
    for( i = 0; i < point_count; i++ )
    {
        int idx = 0, on_edge = 0;
        double pptresult = cvTsPointPolygonTest( p[i], h, hull_count, &idx, &on_edge );

        if( pptresult < 0 )
        {
            ts->printf( cvtest::TS::LOG, "The point #%d is outside of the convex hull\n", i );
            code = cvtest::TS::FAIL_BAD_ACCURACY;
            goto _exit_;
        }

        if( pptresult < FLT_EPSILON && !on_edge )
            mask->data.ptr[idx] = (uchar)1;
    }

    if( cvtest::norm( _mask, Mat::zeros(_mask.dims, _mask.size, _mask.type()), NORM_L1 ) != hull_count )
    {
        ts->printf( cvtest::TS::LOG, "Not every convex hull vertex coincides with some input point\n" );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

_exit_:

    cvReleaseMat( &hull );
    cvReleaseMat( &mask );
    if( code < 0 )
        ts->set_failed_test_info( code );
    return code;
}


/****************************************************************************************\
*                                     MinAreaRect Test                                   *
\****************************************************************************************/

class CV_MinAreaRectTest : public CV_BaseShapeDescrTest
{
public:
    CV_MinAreaRectTest();

protected:
    void run_func(void);
    int validate_test_results( int test_case_idx );

    CvBox2D box;
    CvPoint2D32f box_pt[4];
};


CV_MinAreaRectTest::CV_MinAreaRectTest()
{
}


void CV_MinAreaRectTest::run_func()
{
    if(!test_cpp)
    {
        box = cvMinAreaRect2( points, storage );
        cvBoxPoints( box, box_pt );
    }
    else
    {
        cv::RotatedRect r = cv::minAreaRect(cv::cvarrToMat(points));
        box = cvBox2D(r);
        r.points((cv::Point2f*)box_pt);
    }
}


int CV_MinAreaRectTest::validate_test_results( int test_case_idx )
{
    double eps = 1e-1;
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    int i, j, point_count = points2->rows + points2->cols - 1;
    CvPoint2D32f *p = (CvPoint2D32f*)(points2->data.ptr);
    int mask[] = {0,0,0,0};

    // check that the bounding box is a rotated rectangle:
    //  1. diagonals should be equal
    //  2. they must intersect in their middle points
    {
        double d0 = cvTsDist( box_pt[0], box_pt[2] );
        double d1 = cvTsDist( box_pt[1], box_pt[3] );

        double x0 = (box_pt[0].x + box_pt[2].x)*0.5;
        double y0 = (box_pt[0].y + box_pt[2].y)*0.5;
        double x1 = (box_pt[1].x + box_pt[3].x)*0.5;
        double y1 = (box_pt[1].y + box_pt[3].y)*0.5;

        if( fabs(d0 - d1) + fabs(x0 - x1) + fabs(y0 - y1) > eps*MAX(d0,d1) )
        {
            ts->printf( cvtest::TS::LOG, "The bounding box is not a rectangle\n" );
            code = cvtest::TS::FAIL_INVALID_OUTPUT;
            goto _exit_;
        }
    }

#if 0
    {
    int n = 4;
    double a = 8, c = 8, b = 100, d = 150;
    CvPoint bp[4], *bpp = bp;
    cvNamedWindow( "test", 1 );
    IplImage* img = cvCreateImage( cvSize(500,500), 8, 3 );
    cvZero(img);
    for( i = 0; i < point_count; i++ )
        cvCircle(img,cvPoint(cvRound(p[i].x*a+b),cvRound(p[i].y*c+d)), 3, CV_RGB(0,255,0), -1 );
    for( i = 0; i < n; i++ )
        bp[i] = cvPoint(cvRound(box_pt[i].x*a+b),cvRound(box_pt[i].y*c+d));
    cvPolyLine( img, &bpp, &n, 1, 1, CV_RGB(255,255,0), 1, CV_AA, 0 );
    cvShowImage( "test", img );
    cvWaitKey();
    cvReleaseImage(&img);
    }
#endif

    // check that the box includes all the points
    // and there is at least one point at (or very close to) every box side
    for( i = 0; i < point_count; i++ )
    {
        int idx = 0, on_edge = 0;
        double pptresult = cvTsPointPolygonTest( p[i], box_pt, 4, &idx, &on_edge );
        if( pptresult < -eps )
        {
            ts->printf( cvtest::TS::LOG, "The point #%d is outside of the box\n", i );
            code = cvtest::TS::FAIL_BAD_ACCURACY;
            goto _exit_;
        }

        if( pptresult < eps )
        {
            for( j = 0; j < 4; j++ )
            {
                double d = cvTsPtLineDist( p[i], box_pt[(j-1)&3], box_pt[j] );
                if( d < eps )
                    mask[j] = (uchar)1;
            }
        }
    }

    if( mask[0] + mask[1] + mask[2] + mask[3] != 4 )
    {
        ts->printf( cvtest::TS::LOG, "Not every box side has a point nearby\n" );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

_exit_:

    if( code < 0 )
        ts->set_failed_test_info( code );
    return code;
}


/****************************************************************************************\
*                                   MinEnclosingTriangle Test                            *
\****************************************************************************************/

class CV_MinTriangleTest : public CV_BaseShapeDescrTest
{
public:
    CV_MinTriangleTest();

protected:
    void run_func(void);
    int validate_test_results( int test_case_idx );
    std::vector<cv::Point2f> getTriangleMiddlePoints();

    std::vector<cv::Point2f> convexPolygon;
    std::vector<cv::Point2f> triangle;
};


CV_MinTriangleTest::CV_MinTriangleTest()
{
}

std::vector<cv::Point2f> CV_MinTriangleTest::getTriangleMiddlePoints()
{
    std::vector<cv::Point2f> triangleMiddlePoints;

    for (int i = 0; i < 3; i++) {
        triangleMiddlePoints.push_back(cvTsMiddlePoint(triangle[i], triangle[(i + 1) % 3]));
    }

    return triangleMiddlePoints;
}


void CV_MinTriangleTest::run_func()
{
    std::vector<cv::Point2f> pointsAsVector;

    cv::cvarrToMat(points).convertTo(pointsAsVector, CV_32F);

    cv::minEnclosingTriangle(pointsAsVector, triangle);
    cv::convexHull(pointsAsVector, convexPolygon, true, true);
}


int CV_MinTriangleTest::validate_test_results( int test_case_idx )
{
    bool errorEnclosed = false, errorMiddlePoints = false, errorFlush = true;
    double eps = 1e-4;
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );

#if 0
    {
    int n = 3;
    double a = 8, c = 8, b = 100, d = 150;
    CvPoint bp[4], *bpp = bp;
    cvNamedWindow( "test", 1 );
    IplImage* img = cvCreateImage( cvSize(500,500), 8, 3 );
    cvZero(img);
    for( i = 0; i < point_count; i++ )
        cvCircle(img,cvPoint(cvRound(p[i].x*a+b),cvRound(p[i].y*c+d)), 3, CV_RGB(0,255,0), -1 );
    for( i = 0; i < n; i++ )
        bp[i] = cvPoint(cvRound(triangle[i].x*a+b),cvRound(triangle[i].y*c+d));
    cvPolyLine( img, &bpp, &n, 1, 1, CV_RGB(255,255,0), 1, CV_AA, 0 );
    cvShowImage( "test", img );
    cvWaitKey();
    cvReleaseImage(&img);
    }
#endif

    int polygonVertices = (int) convexPolygon.size();

    if (polygonVertices > 2) {
        // Check if all points are enclosed by the triangle
        for (int i = 0; (i < polygonVertices) && (!errorEnclosed); i++)
        {
            if (cv::pointPolygonTest(triangle, cv::Point2f(convexPolygon[i].x, convexPolygon[i].y), true) < (-eps))
                errorEnclosed = true;
        }

        // Check if triangle edges middle points touch the polygon
        std::vector<cv::Point2f> middlePoints = getTriangleMiddlePoints();

        for (int i = 0; (i < 3) && (!errorMiddlePoints); i++)
        {
            bool isTouching = false;

            for (int j = 0; (j < polygonVertices) && (!isTouching); j++)
            {
                if (cvTsIsPointOnLineSegment(middlePoints[i], convexPolygon[j],
                                             convexPolygon[(j + 1) % polygonVertices]))
                    isTouching = true;
            }

            errorMiddlePoints = (isTouching) ? false : true;
        }

        // Check if at least one of the edges is flush
        for (int i = 0; (i < 3) && (errorFlush); i++)
        {
            for (int j = 0; (j < polygonVertices) && (errorFlush); j++)
            {
                if ((cvTsIsPointOnLineSegment(convexPolygon[j], triangle[i],
                                              triangle[(i + 1) % 3])) &&
                    (cvTsIsPointOnLineSegment(convexPolygon[(j + 1) % polygonVertices], triangle[i],
                                              triangle[(i + 1) % 3])))
                    errorFlush = false;
            }
        }

        // Report any found errors
        if (errorEnclosed)
        {
            ts->printf( cvtest::TS::LOG,
            "All points should be enclosed by the triangle.\n" );
            code = cvtest::TS::FAIL_BAD_ACCURACY;
        }
        else if (errorMiddlePoints)
        {
            ts->printf( cvtest::TS::LOG,
            "All triangle edges middle points should touch the convex hull of the points.\n" );
            code = cvtest::TS::FAIL_INVALID_OUTPUT;
        }
        else if (errorFlush)
        {
            ts->printf( cvtest::TS::LOG,
            "At least one edge of the enclosing triangle should be flush with one edge of the polygon.\n" );
            code = cvtest::TS::FAIL_INVALID_OUTPUT;
        }
    }

    if ( code < 0 )
        ts->set_failed_test_info( code );

    return code;
}


/****************************************************************************************\
*                                     MinEnclosingCircle Test                            *
\****************************************************************************************/

class CV_MinCircleTest : public CV_BaseShapeDescrTest
{
public:
    CV_MinCircleTest();

protected:
    void run_func(void);
    int validate_test_results( int test_case_idx );

    Point2f center;
    float radius;
};


CV_MinCircleTest::CV_MinCircleTest()
{
}


void CV_MinCircleTest::run_func()
{
    if(!test_cpp)
    {
        CvPoint2D32f c_center = cvPoint2D32f(center);
        cvMinEnclosingCircle( points, &c_center, &radius );
        center = c_center;
    }
    else
    {
        cv::Point2f tmpcenter;
        cv::minEnclosingCircle(cv::cvarrToMat(points), tmpcenter, radius);
        center = tmpcenter;
    }
}


int CV_MinCircleTest::validate_test_results( int test_case_idx )
{
    double eps = 1.03;
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    int i, j = 0, point_count = points2->rows + points2->cols - 1;
    Point2f *p = (Point2f*)(points2->data.ptr);
    Point2f v[3];

#if 0
    {
    double a = 2, b = 200, d = 400;
    cvNamedWindow( "test", 1 );
    IplImage* img = cvCreateImage( cvSize(500,500), 8, 3 );
    cvZero(img);
    for( i = 0; i < point_count; i++ )
        cvCircle(img,cvPoint(cvRound(p[i].x*a+b),cvRound(p[i].y*a+d)), 3, CV_RGB(0,255,0), -1 );
    cvCircle( img, cvPoint(cvRound(center.x*a+b),cvRound(center.y*a+d)),
              cvRound(radius*a), CV_RGB(255,255,0), 1 );
    cvShowImage( "test", img );
    cvWaitKey();
    cvReleaseImage(&img);
    }
#endif

    // check that the circle contains all the points inside and
    // remember at most 3 points that are close to the boundary
    for( i = 0; i < point_count; i++ )
    {
        double d = cvTsDist(p[i], center);
        if( d > radius )
        {
            ts->printf( cvtest::TS::LOG, "The point #%d is outside of the circle\n", i );
            code = cvtest::TS::FAIL_BAD_ACCURACY;
            goto _exit_;
        }

        if( radius - d < eps*radius && j < 3 )
            v[j++] = p[i];
    }

    if( point_count >= 2 && (j < 2 || (j == 2 && cvTsDist(v[0],v[1]) < (radius-1)*2/eps)) )
    {
        ts->printf( cvtest::TS::LOG,
            "There should be at at least 3 points near the circle boundary or 2 points on the diameter\n" );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

_exit_:

    if( code < 0 )
        ts->set_failed_test_info( code );
    return code;
}

/****************************************************************************************\
*                                 MinEnclosingCircle Test 2                              *
\****************************************************************************************/

class CV_MinCircleTest2 : public CV_BaseShapeDescrTest
{
public:
    CV_MinCircleTest2();
protected:
    RNG rng;
    void run_func(void);
    int validate_test_results( int test_case_idx );
    float delta;
};


CV_MinCircleTest2::CV_MinCircleTest2()
{
    rng = ts->get_rng();
}


void CV_MinCircleTest2::run_func()
{
    Point2f center = Point2f(rng.uniform(0.0f, 1000.0f), rng.uniform(0.0f, 1000.0f));;
    float radius = rng.uniform(0.0f, 500.0f);
    float angle = (float)rng.uniform(0.0f, (float)(CV_2PI));
    vector<Point2f> pts;
    pts.push_back(center + Point2f(radius * cos(angle), radius * sin(angle)));
    angle += (float)CV_PI;
    pts.push_back(center + Point2f(radius * cos(angle), radius * sin(angle)));
    float radius2 = radius * radius;
    float x = rng.uniform(center.x - radius, center.x + radius);
    float deltaX = x - center.x;
    float upperBoundY = sqrt(radius2 - deltaX * deltaX);
    float y = rng.uniform(center.y - upperBoundY, center.y + upperBoundY);
    pts.push_back(Point2f(x, y));
    // Find the minimum area enclosing circle
    Point2f calcCenter;
    float calcRadius;
    minEnclosingCircle(pts, calcCenter, calcRadius);
    delta = (float)cv::norm(calcCenter - center) + abs(calcRadius - radius);
}

int CV_MinCircleTest2::validate_test_results( int test_case_idx )
{
    float eps = 1.0F;
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    if (delta > eps)
    {
        ts->printf( cvtest::TS::LOG, "Delta center and calcCenter > %f\n", eps );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        ts->set_failed_test_info( code );
    }
    return code;
}

/****************************************************************************************\
*                                 minEnclosingCircle Test 3                              *
\****************************************************************************************/

TEST(Imgproc_minEnclosingCircle, basic_test)
{
    vector<Point2f> pts;
    pts.push_back(Point2f(0, 0));
    pts.push_back(Point2f(10, 0));
    pts.push_back(Point2f(5, 1));
    const float EPS = 1.0e-3f;
    Point2f center;
    float radius;

    // pts[2] is within the circle with diameter pts[0] - pts[1].
    //        2
    // 0             1
    // NB: The triangle is obtuse, so the only pts[0] and pts[1] are on the circle.
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 5, EPS);
    EXPECT_NEAR(center.y, 0, EPS);
    EXPECT_NEAR(5, radius, EPS);

    // pts[2] is on the circle with diameter pts[0] - pts[1].
    //  2
    // 0 1
    pts[2] = Point2f(5, 5);
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 5, EPS);
    EXPECT_NEAR(center.y, 0, EPS);
    EXPECT_NEAR(5, radius, EPS);

    // pts[2] is outside the circle with diameter pts[0] - pts[1].
    //   2
    //
    //
    // 0   1
    // NB: The triangle is acute, so all 3 points are on the circle.
    pts[2] = Point2f(5, 10);
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 5, EPS);
    EXPECT_NEAR(center.y, 3.75, EPS);
    EXPECT_NEAR(6.25f, radius, EPS);

    // The 3 points are colinear.
    pts[2] = Point2f(3, 0);
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 5, EPS);
    EXPECT_NEAR(center.y, 0, EPS);
    EXPECT_NEAR(5, radius, EPS);

    // 2 points are the same.
    pts[2] = pts[1];
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 5, EPS);
    EXPECT_NEAR(center.y, 0, EPS);
    EXPECT_NEAR(5, radius, EPS);

    // 3 points are the same.
    pts[0] = pts[1];
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 10, EPS);
    EXPECT_NEAR(center.y, 0, EPS);
    EXPECT_NEAR(0, radius, EPS);
}

TEST(Imgproc_minEnclosingCircle, regression_16051) {
    vector<Point2f> pts;
    pts.push_back(Point2f(85, 1415));
    pts.push_back(Point2f(87, 1415));
    pts.push_back(Point2f(89, 1414));
    pts.push_back(Point2f(89, 1414));
    pts.push_back(Point2f(87, 1412));
    Point2f center;
    float radius;
    minEnclosingCircle(pts, center, radius);
    EXPECT_NEAR(center.x, 86.9f, 1e-3);
    EXPECT_NEAR(center.y, 1414.1f, 1e-3);
    EXPECT_NEAR(2.1024551f, radius, 1e-3);
}

/****************************************************************************************\
*                                   Perimeter Test                                     *
\****************************************************************************************/

class CV_PerimeterTest : public CV_BaseShapeDescrTest
{
public:
    CV_PerimeterTest();

protected:
    int prepare_test_case( int test_case_idx );
    void run_func(void);
    int validate_test_results( int test_case_idx );
    CvSlice slice;
    int is_closed;
    double result;
};


CV_PerimeterTest::CV_PerimeterTest()
{
}


int CV_PerimeterTest::prepare_test_case( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::prepare_test_case( test_case_idx );
    RNG& rng = ts->get_rng();
    int total;

    if( code < 0 )
        return code;

    is_closed = cvtest::randInt(rng) % 2;

    if( points1 )
    {
        points1->flags |= CV_SEQ_KIND_CURVE;
        if( is_closed )
            points1->flags |= CV_SEQ_FLAG_CLOSED;
        total = points1->total;
    }
    else
        total = points2->cols + points2->rows - 1;

    if( (cvtest::randInt(rng) % 3) && !test_cpp )
    {
        slice.start_index = cvtest::randInt(rng) % total;
        slice.end_index = cvtest::randInt(rng) % total;
    }
    else
        slice = CV_WHOLE_SEQ;

    return 1;
}


void CV_PerimeterTest::run_func()
{
    if(!test_cpp)
        result = cvArcLength( points, slice, points1 ? -1 : is_closed );
    else
        result = cv::arcLength(cv::cvarrToMat(points),
            !points1 ? is_closed != 0 : (points1->flags & CV_SEQ_FLAG_CLOSED) != 0);
}


int CV_PerimeterTest::validate_test_results( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    int i, len = slice.end_index - slice.start_index, total = points2->cols + points2->rows - 1;
    double result0 = 0;
    Point2f prev_pt, pt;
    CvPoint2D32f *ptr;

    if( len < 0 )
        len += total;

    len = MIN( len, total );
    //len -= !is_closed && len == total;

    ptr = (CvPoint2D32f*)points2->data.fl;
    prev_pt = ptr[(is_closed ? slice.start_index+len-1 : slice.start_index) % total];

    for( i = 0; i < len + (len < total && (!is_closed || len==1)); i++ )
    {
        pt = ptr[(i + slice.start_index) % total];
        double dx = pt.x - prev_pt.x, dy = pt.y - prev_pt.y;
        result0 += sqrt(dx*dx + dy*dy);
        prev_pt = pt;
    }

    if( cvIsNaN(result) || cvIsInf(result) )
    {
        ts->printf( cvtest::TS::LOG, "cvArcLength() returned invalid value (%g)\n", result );
        code = cvtest::TS::FAIL_INVALID_OUTPUT;
    }
    else if( fabs(result - result0) > FLT_EPSILON*100*result0 )
    {
        ts->printf( cvtest::TS::LOG, "The function returned %g, while the correct result is %g\n", result, result0 );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
    }

    if( code < 0 )
        ts->set_failed_test_info( code );
    return code;
}


/****************************************************************************************\
*                                   FitEllipse Test                                      *
\****************************************************************************************/

class CV_FitEllipseTest : public CV_BaseShapeDescrTest
{
public:
    CV_FitEllipseTest();

protected:
    int prepare_test_case( int test_case_idx );
    void generate_point_set( void* points );
    void run_func(void);
    int validate_test_results( int test_case_idx );
    RotatedRect box0, box;
    double min_ellipse_size, max_noise;
};


CV_FitEllipseTest::CV_FitEllipseTest()
{
    min_log_size = 5; // for robust ellipse fitting a dozen of points is needed at least
    max_log_size = 10;
    min_ellipse_size = 10;
    max_noise = 0.05;
}


void CV_FitEllipseTest::generate_point_set( void* pointsSet )
{
    RNG& rng = ts->get_rng();
    int i, total, point_type;
    CvSeqReader reader;
    uchar* data = 0;
    double a, b;

    box0.center.x = (float)((low.val[0] + high.val[0])*0.5);
    box0.center.y = (float)((low.val[1] + high.val[1])*0.5);
    box0.size.width = (float)(MAX(high.val[0] - low.val[0], min_ellipse_size)*2);
    box0.size.height = (float)(MAX(high.val[1] - low.val[1], min_ellipse_size)*2);
    box0.angle = (float)(cvtest::randReal(rng)*180);
    a = cos(box0.angle*CV_PI/180.);
    b = sin(box0.angle*CV_PI/180.);

    if( box0.size.width > box0.size.height )
    {
        float t;
        CV_SWAP( box0.size.width, box0.size.height, t );
    }
    memset( &reader, 0, sizeof(reader) );

    if( CV_IS_SEQ(pointsSet) )
    {
        CvSeq* ptseq = (CvSeq*)pointsSet;
        total = ptseq->total;
        point_type = CV_SEQ_ELTYPE(ptseq);
        cvStartReadSeq( ptseq, &reader );
    }
    else
    {
        CvMat* ptm = (CvMat*)pointsSet;
        CV_Assert( CV_IS_MAT(ptm) && CV_IS_MAT_CONT(ptm->type) );
        total = ptm->rows + ptm->cols - 1;
        point_type = CV_MAT_TYPE(ptm->type);
        data = ptm->data.ptr;
    }

    CV_Assert(point_type == CV_32SC2 || point_type == CV_32FC2);

    for( i = 0; i < total; i++ )
    {
        CvPoint* pp;
        CvPoint2D32f p = {0, 0};
        double angle = cvtest::randReal(rng)*CV_PI*2;
        double x = box0.size.height*0.5*(cos(angle) + (cvtest::randReal(rng)-0.5)*2*max_noise);
        double y = box0.size.width*0.5*(sin(angle) + (cvtest::randReal(rng)-0.5)*2*max_noise);
        p.x = (float)(box0.center.x + a*x + b*y);
        p.y = (float)(box0.center.y - b*x + a*y);

        if( reader.ptr )
        {
            pp = (CvPoint*)reader.ptr;
            CV_NEXT_SEQ_ELEM( sizeof(*pp), reader );
        }
        else
            pp = ((CvPoint*)data) + i;
        if( point_type == CV_32SC2 )
        {
            pp->x = cvRound(p.x);
            pp->y = cvRound(p.y);
        }
        else
            *(CvPoint2D32f*)pp = p;
    }
}


int CV_FitEllipseTest::prepare_test_case( int test_case_idx )
{
    min_log_size = MAX(min_log_size,4);
    max_log_size = MAX(min_log_size,max_log_size);
    return CV_BaseShapeDescrTest::prepare_test_case( test_case_idx );
}


void CV_FitEllipseTest::run_func()
{
    if(!test_cpp)
        box = cvFitEllipse2( points );
    else
        box = cv::fitEllipse(cv::cvarrToMat(points));
}

int CV_FitEllipseTest::validate_test_results( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    double diff_angle;

    if( cvIsNaN(box.center.x) || cvIsInf(box.center.x) ||
        cvIsNaN(box.center.y) || cvIsInf(box.center.y) ||
        cvIsNaN(box.size.width) || cvIsInf(box.size.width) ||
        cvIsNaN(box.size.height) || cvIsInf(box.size.height) ||
        cvIsNaN(box.angle) || cvIsInf(box.angle) )
    {
        ts->printf( cvtest::TS::LOG, "Some of the computed ellipse parameters are invalid (x=%g,y=%g,w=%g,h=%g,angle=%g)\n",
            box.center.x, box.center.y, box.size.width, box.size.height, box.angle );
        code = cvtest::TS::FAIL_INVALID_OUTPUT;
        goto _exit_;
    }

    box.angle = (float)(90-box.angle);
    if( box.angle < 0 )
        box.angle += 360;
    if( box.angle > 360 )
        box.angle -= 360;

    if( fabs(box.center.x - box0.center.x) > 3 ||
        fabs(box.center.y - box0.center.y) > 3 ||
        fabs(box.size.width - box0.size.width) > 0.1*fabs(box0.size.width) ||
        fabs(box.size.height - box0.size.height) > 0.1*fabs(box0.size.height) )
    {
        ts->printf( cvtest::TS::LOG, "The computed ellipse center and/or size are incorrect:\n\t"
            "(x=%.1f,y=%.1f,w=%.1f,h=%.1f), while it should be (x=%.1f,y=%.1f,w=%.1f,h=%.1f)\n",
            box.center.x, box.center.y, box.size.width, box.size.height,
            box0.center.x, box0.center.y, box0.size.width, box0.size.height );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

    diff_angle = fabs(box0.angle - box.angle);
    diff_angle = MIN( diff_angle, fabs(diff_angle - 360));
    diff_angle = MIN( diff_angle, fabs(diff_angle - 180));

    if( box0.size.height >= 1.3*box0.size.width && diff_angle > 30 )
    {
        ts->printf( cvtest::TS::LOG, "Incorrect ellipse angle (=%1.f, should be %1.f)\n",
            box.angle, box0.angle );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

_exit_:

#if 0
    if( code < 0 )
    {
    cvNamedWindow( "test", 0 );
    IplImage* img = cvCreateImage( cvSize(cvRound(low_high_range*4),
        cvRound(low_high_range*4)), 8, 3 );
    cvZero( img );

    box.center.x += (float)low_high_range*2;
    box.center.y += (float)low_high_range*2;
    cvEllipseBox( img, box, CV_RGB(255,0,0), 3, 8 );

    for( int i = 0; i < points2->rows + points2->cols - 1; i++ )
    {
        CvPoint pt;
        pt.x = cvRound(points2->data.fl[i*2] + low_high_range*2);
        pt.y = cvRound(points2->data.fl[i*2+1] + low_high_range*2);
        cvCircle( img, pt, 1, CV_RGB(255,255,255), -1, 8 );
    }

    cvShowImage( "test", img );
    cvReleaseImage( &img );
    cvWaitKey(0);
    }
#endif

    if( code < 0 )
    {
        ts->set_failed_test_info( code );
    }
    return code;
}


class CV_FitEllipseSmallTest : public cvtest::BaseTest
{
public:
    CV_FitEllipseSmallTest() {}
    ~CV_FitEllipseSmallTest() {}
protected:
    void run(int)
    {
        Size sz(50, 50);
        vector<vector<Point> > c;
        c.push_back(vector<Point>());
        int scale = 1;
        Point ofs = Point(0,0);//sz.width/2, sz.height/2) - Point(4,4)*scale;
        c[0].push_back(Point(2, 0)*scale+ofs);
        c[0].push_back(Point(0, 2)*scale+ofs);
        c[0].push_back(Point(0, 6)*scale+ofs);
        c[0].push_back(Point(2, 8)*scale+ofs);
        c[0].push_back(Point(6, 8)*scale+ofs);
        c[0].push_back(Point(8, 6)*scale+ofs);
        c[0].push_back(Point(8, 2)*scale+ofs);
        c[0].push_back(Point(6, 0)*scale+ofs);

        RotatedRect e = fitEllipse(c[0]);
        CV_Assert( fabs(e.center.x - 4) <= 1. &&
                   fabs(e.center.y - 4) <= 1. &&
                   fabs(e.size.width - 9) <= 1. &&
                   fabs(e.size.height - 9) <= 1. );
    }
};


// Regression test for incorrect fitEllipse result reported in Bug #3989
// Check edge cases for rotation angles of ellipse ([-180, 90, 0, 90, 180] degrees)
class CV_FitEllipseParallelTest : public CV_FitEllipseTest
{
public:
    CV_FitEllipseParallelTest();
    ~CV_FitEllipseParallelTest();
protected:
    void generate_point_set( void* points );
    void run_func(void);
    Mat pointsMat;
};

CV_FitEllipseParallelTest::CV_FitEllipseParallelTest()
{
    min_ellipse_size = 5;
}

void CV_FitEllipseParallelTest::generate_point_set( void* )
{
    RNG& rng = ts->get_rng();
    int height = (int)(MAX(high.val[0] - low.val[0], min_ellipse_size));
    int width = (int)(MAX(high.val[1] - low.val[1], min_ellipse_size));
    const int angle = ( (cvtest::randInt(rng) % 5) - 2 ) * 90;
    const int dim = max(height, width);
    const Point center = Point(dim*2, dim*2);

    if( width > height )
    {
        int t;
        CV_SWAP( width, height, t );
    }

    Mat image = Mat::zeros(dim*4, dim*4, CV_8UC1);
    ellipse(image, center, Size(height, width), angle,
            0, 360, Scalar(255, 0, 0), 1, 8);

    box0.center.x = (float)center.x;
    box0.center.y = (float)center.y;
    box0.size.width = (float)width*2;
    box0.size.height = (float)height*2;
    box0.angle = (float)angle;

    vector<vector<Point> > contours;
    findContours(image, contours,  RETR_EXTERNAL,  CHAIN_APPROX_NONE);
    Mat(contours[0]).convertTo(pointsMat, CV_32F);
}

void CV_FitEllipseParallelTest::run_func()
{
    box = cv::fitEllipse(pointsMat);
}

CV_FitEllipseParallelTest::~CV_FitEllipseParallelTest(){
    pointsMat.release();
}

/****************************************************************************************\
*                                   FitLine Test                                         *
\****************************************************************************************/

class CV_FitLineTest : public CV_BaseShapeDescrTest
{
public:
    CV_FitLineTest();

protected:
    int prepare_test_case( int test_case_idx );
    void generate_point_set( void* points );
    void run_func(void);
    int validate_test_results( int test_case_idx );
    double max_noise;
    AutoBuffer<float> line, line0;
    int dist_type;
    double reps, aeps;
};


CV_FitLineTest::CV_FitLineTest()
{
    min_log_size = 5; // for robust line fitting a dozen of points is needed at least
    max_log_size = 10;
    max_noise = 0.05;
}

void CV_FitLineTest::generate_point_set( void* pointsSet )
{
    RNG& rng = ts->get_rng();
    int i, k, n, total, point_type;
    CvSeqReader reader;
    uchar* data = 0;
    double s = 0;

    n = dims;
    for( k = 0; k < n; k++ )
    {
        line0[k+n] = (float)((low.val[k] + high.val[k])*0.5);
        line0[k] = (float)(high.val[k] - low.val[k]);
        if( cvtest::randInt(rng) % 2 )
            line0[k] = -line0[k];
        s += (double)line0[k]*line0[k];
    }

    s = 1./sqrt(s);
    for( k = 0; k < n; k++ )
        line0[k] = (float)(line0[k]*s);

    memset( &reader, 0, sizeof(reader) );

    if( CV_IS_SEQ(pointsSet) )
    {
        CvSeq* ptseq = (CvSeq*)pointsSet;
        total = ptseq->total;
        point_type = CV_MAT_DEPTH(CV_SEQ_ELTYPE(ptseq));
        cvStartReadSeq( ptseq, &reader );
    }
    else
    {
        CvMat* ptm = (CvMat*)pointsSet;
        CV_Assert( CV_IS_MAT(ptm) && CV_IS_MAT_CONT(ptm->type) );
        total = ptm->rows + ptm->cols - 1;
        point_type = CV_MAT_DEPTH(CV_MAT_TYPE(ptm->type));
        data = ptm->data.ptr;
    }

    for( i = 0; i < total; i++ )
    {
        int* pi;
        float* pf;
        float p[4], t;
        if( reader.ptr )
        {
            pi = (int*)reader.ptr;
            pf = (float*)reader.ptr;
            CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
        }
        else
        {
            pi = (int*)data + i*n;
            pf = (float*)data + i*n;
        }

        t = (float)((cvtest::randReal(rng)-0.5)*low_high_range*2);

        for( k = 0; k < n; k++ )
        {
            p[k] = (float)((cvtest::randReal(rng)-0.5)*max_noise*2 + t*line0[k] + line0[k+n]);

            if( point_type == CV_32S )
                pi[k] = cvRound(p[k]);
            else
                pf[k] = p[k];
        }
    }
}

int CV_FitLineTest::prepare_test_case( int test_case_idx )
{
    RNG& rng = ts->get_rng();
    dims = cvtest::randInt(rng) % 2 + 2;
    line.allocate(dims * 2);
    line0.allocate(dims * 2);
    min_log_size = MAX(min_log_size,5);
    max_log_size = MAX(min_log_size,max_log_size);
    int code = CV_BaseShapeDescrTest::prepare_test_case( test_case_idx );
    dist_type = cvtest::randInt(rng) % 6 + 1;
    dist_type += dist_type == CV_DIST_C;
    reps = 0.1; aeps = 0.01;
    return code;
}


void CV_FitLineTest::run_func()
{
    if(!test_cpp)
        cvFitLine( points, dist_type, 0, reps, aeps, line.data());
    else if(dims == 2)
        cv::fitLine(cv::cvarrToMat(points), (cv::Vec4f&)line[0], dist_type, 0, reps, aeps);
    else
        cv::fitLine(cv::cvarrToMat(points), (cv::Vec6f&)line[0], dist_type, 0, reps, aeps);
}

int CV_FitLineTest::validate_test_results( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    int k, max_k = 0;
    double vec_diff = 0, t;

    //std::cout << dims << " " << Mat(1, dims*2, CV_32FC1, line.data()) << " " << Mat(1, dims, CV_32FC1, line0.data()) << std::endl;

    for( k = 0; k < dims*2; k++ )
    {
        if( cvIsNaN(line[k]) || cvIsInf(line[k]) )
        {
            ts->printf( cvtest::TS::LOG, "Some of the computed line parameters are invalid (line[%d]=%g)\n",
                k, line[k] );
            code = cvtest::TS::FAIL_INVALID_OUTPUT;
            goto _exit_;
        }
    }

    if( fabs(line0[1]) > fabs(line0[0]) )
        max_k = 1;
    if( fabs(line0[dims-1]) > fabs(line0[max_k]) )
        max_k = dims-1;
    if( line0[max_k] < 0 )
        for( k = 0; k < dims; k++ )
            line0[k] = -line0[k];
    if( line[max_k] < 0 )
        for( k = 0; k < dims; k++ )
            line[k] = -line[k];

    for( k = 0; k < dims; k++ )
    {
        double dt = line[k] - line0[k];
        vec_diff += dt*dt;
    }

    if( sqrt(vec_diff) > 0.05 )
    {
        if( dims == 2 )
            ts->printf( cvtest::TS::LOG,
                "The computed line vector (%.2f,%.2f) is different from the actual (%.2f,%.2f)\n",
                line[0], line[1], line0[0], line0[1] );
        else
            ts->printf( cvtest::TS::LOG,
                "The computed line vector (%.2f,%.2f,%.2f) is different from the actual (%.2f,%.2f,%.2f)\n",
                line[0], line[1], line[2], line0[0], line0[1], line0[2] );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

    t = (line[max_k+dims] - line0[max_k+dims])/line0[max_k];
    for( k = 0; k < dims; k++ )
    {
        double p = line0[k+dims] + t*line0[k] - line[k+dims];
        vec_diff += p*p;
    }

    if( sqrt(vec_diff) > 1*MAX(fabs(t),1) )
    {
        if( dims == 2 )
            ts->printf( cvtest::TS::LOG,
                "The computed line point (%.2f,%.2f) is too far from the actual line\n",
                line[2]+line0[2], line[3]+line0[3] );
        else
            ts->printf( cvtest::TS::LOG,
                "The computed line point (%.2f,%.2f,%.2f) is too far from the actual line\n",
                line[3]+line0[3], line[4]+line0[4], line[5]+line0[5] );
        code = cvtest::TS::FAIL_BAD_ACCURACY;
        goto _exit_;
    }

_exit_:

    if( code < 0 )
    {
        ts->set_failed_test_info( code );
    }
    return code;
}

/****************************************************************************************\
*                                   ContourMoments Test                                  *
\****************************************************************************************/


static void
cvTsGenerateTousledBlob( CvPoint2D32f center, CvSize2D32f axes,
    double max_r_scale, double angle, CvArr* points, RNG& rng )
{
    int i, total, point_type;
    uchar* data = 0;
    CvSeqReader reader;
    memset( &reader, 0, sizeof(reader) );

    if( CV_IS_SEQ(points) )
    {
        CvSeq* ptseq = (CvSeq*)points;
        total = ptseq->total;
        point_type = CV_SEQ_ELTYPE(ptseq);
        cvStartReadSeq( ptseq, &reader );
    }
    else
    {
        CvMat* ptm = (CvMat*)points;
        CV_Assert( CV_IS_MAT(ptm) && CV_IS_MAT_CONT(ptm->type) );
        total = ptm->rows + ptm->cols - 1;
        point_type = CV_MAT_TYPE(ptm->type);
        data = ptm->data.ptr;
    }

    CV_Assert( point_type == CV_32SC2 || point_type == CV_32FC2 );

    for( i = 0; i < total; i++ )
    {
        CvPoint* pp;
        Point2f p;

        double phi0 = 2*CV_PI*i/total;
        double phi = CV_PI*angle/180.;
        double t = cvtest::randReal(rng)*max_r_scale + (1 - max_r_scale);
        double ta = axes.height*t;
        double tb = axes.width*t;
        double c0 = cos(phi0)*ta, s0 = sin(phi0)*tb;
        double c = cos(phi), s = sin(phi);
        p.x = (float)(c0*c - s0*s + center.x);
        p.y = (float)(c0*s + s0*c + center.y);

        if( reader.ptr )
        {
            pp = (CvPoint*)reader.ptr;
            CV_NEXT_SEQ_ELEM( sizeof(*pp), reader );
        }
        else
            pp = ((CvPoint*)data) + i;

        if( point_type == CV_32SC2 )
        {
            pp->x = cvRound(p.x);
            pp->y = cvRound(p.y);
        }
        else
            *(CvPoint2D32f*)pp = cvPoint2D32f(p);
    }
}


class CV_ContourMomentsTest : public CV_BaseShapeDescrTest
{
public:
    CV_ContourMomentsTest();

protected:
    int prepare_test_case( int test_case_idx );
    void generate_point_set( void* points );
    void run_func(void);
    int validate_test_results( int test_case_idx );
    CvMoments moments0, moments;
    double area0, area;
    Size2f axes;
    Point2f center;
    int max_max_r_scale;
    double max_r_scale, angle;
    Size img_size;
};


CV_ContourMomentsTest::CV_ContourMomentsTest()
{
    min_log_size = 3;
    max_log_size = 8;
    max_max_r_scale = 15;
    low_high_range = 200;
    enable_flt_points = false;
}


void CV_ContourMomentsTest::generate_point_set( void* pointsSet )
{
    RNG& rng = ts->get_rng();
    float max_sz;

    axes.width = (float)((cvtest::randReal(rng)*0.9 + 0.1)*low_high_range);
    axes.height = (float)((cvtest::randReal(rng)*0.9 + 0.1)*low_high_range);
    max_sz = MAX(axes.width, axes.height);

    img_size.width = img_size.height = cvRound(low_high_range*2.2);

    center.x = (float)(img_size.width*0.5 + (cvtest::randReal(rng)-0.5)*(img_size.width - max_sz*2)*0.8);
    center.y = (float)(img_size.height*0.5 + (cvtest::randReal(rng)-0.5)*(img_size.height - max_sz*2)*0.8);

    CV_Assert( 0 < center.x - max_sz && center.x + max_sz < img_size.width &&
               0 < center.y - max_sz && center.y + max_sz < img_size.height );

    max_r_scale = cvtest::randReal(rng)*max_max_r_scale*0.01;
    angle = cvtest::randReal(rng)*360;

    cvTsGenerateTousledBlob( cvPoint2D32f(center), cvSize2D32f(axes), max_r_scale, angle, pointsSet, rng );

    if( points1 )
        points1->flags = CV_SEQ_MAGIC_VAL + CV_SEQ_POLYGON;
}


int CV_ContourMomentsTest::prepare_test_case( int test_case_idx )
{
    min_log_size = MAX(min_log_size,3);
    max_log_size = MIN(max_log_size,8);
    max_log_size = MAX(min_log_size,max_log_size);
    int code = CV_BaseShapeDescrTest::prepare_test_case( test_case_idx );
    return code;
}


void CV_ContourMomentsTest::run_func()
{
    if(!test_cpp)
    {
        cvMoments( points, &moments );
        area = cvContourArea( points );
    }
    else
    {
        moments = cvMoments(cv::moments(cv::cvarrToMat(points)));
        area = cv::contourArea(cv::cvarrToMat(points));
    }
}


int CV_ContourMomentsTest::validate_test_results( int test_case_idx )
{
    int code = CV_BaseShapeDescrTest::validate_test_results( test_case_idx );
    int i, n = (int)(sizeof(moments)/sizeof(moments.inv_sqrt_m00));
    CvMat* img = cvCreateMat( img_size.height, img_size.width, CV_8UC1 );
    CvPoint* pt = (CvPoint*)points2->data.i;
    int count = points2->cols + points2->rows - 1;
    double max_v0 = 0;

    cvZero(img);
    cvFillPoly( img, &pt, &count, 1, cvScalarAll(1));
    cvMoments( img, &moments0 );

    for( i = 0; i < n; i++ )
    {
        double t = fabs((&moments0.m00)[i]);
        max_v0 = MAX(max_v0, t);
    }

    for( i = 0; i <= n; i++ )
    {
        double v = i < n ? (&moments.m00)[i] : area;
        double v0 = i < n ? (&moments0.m00)[i] : moments0.m00;

        if( cvIsNaN(v) || cvIsInf(v) )
        {
            ts->printf( cvtest::TS::LOG,
                "The contour %s is invalid (=%g)\n", i < n ? "moment" : "area", v );
            code = cvtest::TS::FAIL_INVALID_OUTPUT;
            break;
        }

        if( fabs(v - v0) > 0.1*max_v0 )
        {
            ts->printf( cvtest::TS::LOG,
                "The computed contour %s is %g, while it should be %g\n",
                i < n ? "moment" : "area", v, v0 );
            code = cvtest::TS::FAIL_BAD_ACCURACY;
            break;
        }
    }

    if( code < 0 )
    {
#if 0
        cvCmpS( img, 0, img, CV_CMP_GT );
        cvNamedWindow( "test", 1 );
        cvShowImage( "test", img );
        cvWaitKey();
#endif
        ts->set_failed_test_info( code );
    }

    cvReleaseMat( &img );
    return code;
}


////////////////////////////////////// Perimeter/Area/Slice test ///////////////////////////////////

class CV_PerimeterAreaSliceTest : public cvtest::BaseTest
{
public:
    CV_PerimeterAreaSliceTest();
    ~CV_PerimeterAreaSliceTest();
protected:
    void run(int);
};

CV_PerimeterAreaSliceTest::CV_PerimeterAreaSliceTest()
{
}
CV_PerimeterAreaSliceTest::~CV_PerimeterAreaSliceTest() {}

void CV_PerimeterAreaSliceTest::run( int )
{
    Ptr<CvMemStorage> storage(cvCreateMemStorage());
    RNG& rng = theRNG();
    const double min_r = 90, max_r = 120;

    for( int i = 0; i < 100; i++ )
    {
        ts->update_context( this, i, true );
        int n = rng.uniform(3, 30);
        cvClearMemStorage(storage);
        CvSeq* contour = cvCreateSeq(CV_SEQ_POLYGON, sizeof(CvSeq), sizeof(CvPoint), storage);
        double dphi = CV_PI*2/n;
        Point center;
        center.x = rng.uniform(cvCeil(max_r), cvFloor(640-max_r));
        center.y = rng.uniform(cvCeil(max_r), cvFloor(480-max_r));

        for( int j = 0; j < n; j++ )
        {
            CvPoint pt = CV_STRUCT_INITIALIZER;
            double r = rng.uniform(min_r, max_r);
            double phi = j*dphi;
            pt.x = cvRound(center.x + r*cos(phi));
            pt.y = cvRound(center.y - r*sin(phi));
            cvSeqPush(contour, &pt);
        }

        CvSlice slice = {0, 0};
        for(;;)
        {
            slice.start_index = rng.uniform(-n/2, 3*n/2);
            slice.end_index = rng.uniform(-n/2, 3*n/2);
            int len = cvSliceLength(slice, contour);
            if( len > 2 )
                break;
        }
        CvSeq *cslice = cvSeqSlice(contour, slice);
        /*printf( "%d. (%d, %d) of %d, length = %d, length1 = %d\n",
               i, slice.start_index, slice.end_index,
               contour->total, cvSliceLength(slice, contour), cslice->total );

        double area0 = cvContourArea(cslice);
        double area1 = cvContourArea(contour, slice);
        if( area0 != area1 )
        {
            ts->printf(cvtest::TS::LOG,
                       "The contour area slice is computed differently (%g vs %g)\n", area0, area1 );
            ts->set_failed_test_info( cvtest::TS::FAIL_BAD_ACCURACY );
            return;
        }*/

        double len0 = cvArcLength(cslice, CV_WHOLE_SEQ, 1);
        double len1 = cvArcLength(contour, slice, 1);
        if( len0 != len1 )
        {
            ts->printf(cvtest::TS::LOG,
                       "The contour arc length is computed differently (%g vs %g)\n", len0, len1 );
            ts->set_failed_test_info( cvtest::TS::FAIL_BAD_ACCURACY );
            return;
        }
    }
    ts->set_failed_test_info(cvtest::TS::OK);
}


TEST(Imgproc_ConvexHull, accuracy) { CV_ConvHullTest test; test.safe_run(); }
TEST(Imgproc_MinAreaRect, accuracy) { CV_MinAreaRectTest test; test.safe_run(); }
TEST(Imgproc_MinTriangle, accuracy) { CV_MinTriangleTest test; test.safe_run(); }
TEST(Imgproc_MinCircle, accuracy) { CV_MinCircleTest test; test.safe_run(); }
TEST(Imgproc_MinCircle2, accuracy) { CV_MinCircleTest2 test; test.safe_run(); }
TEST(Imgproc_ContourPerimeter, accuracy) { CV_PerimeterTest test; test.safe_run(); }
TEST(Imgproc_FitEllipse, accuracy) { CV_FitEllipseTest test; test.safe_run(); }
TEST(Imgproc_FitEllipse, parallel) { CV_FitEllipseParallelTest test; test.safe_run(); }
TEST(Imgproc_FitLine, accuracy) { CV_FitLineTest test; test.safe_run(); }
TEST(Imgproc_ContourMoments, accuracy) { CV_ContourMomentsTest test; test.safe_run(); }
TEST(Imgproc_ContourPerimeterSlice, accuracy) { CV_PerimeterAreaSliceTest test; test.safe_run(); }
TEST(Imgproc_FitEllipse, small) { CV_FitEllipseSmallTest test; test.safe_run(); }



PARAM_TEST_CASE(ConvexityDefects_regression_5908, bool, int)
{
public:
    int start_index;
    bool clockwise;

    Mat contour;

    virtual void SetUp()
    {
        clockwise = GET_PARAM(0);
        start_index = GET_PARAM(1);

        const int N = 11;
        const Point2i points[N] = {
            Point2i(154, 408),
            Point2i(45, 223),
            Point2i(115, 275), // inner
            Point2i(104, 166),
            Point2i(154, 256), // inner
            Point2i(169, 144),
            Point2i(185, 256), // inner
            Point2i(235, 170),
            Point2i(240, 320), // inner
            Point2i(330, 287),
            Point2i(224, 390)
        };

        contour = Mat(N, 1, CV_32SC2);
        for (int i = 0; i < N; i++)
        {
            contour.at<Point2i>(i) = (!clockwise) // image and convexHull coordinate systems are different
                    ? points[(start_index + i) % N]
                    : points[N - 1 - ((start_index + i) % N)];
        }
    }
};

TEST_P(ConvexityDefects_regression_5908, simple)
{
    std::vector<int> hull;
    cv::convexHull(contour, hull, clockwise, false);

    std::vector<Vec4i> result;
    cv::convexityDefects(contour, hull, result);

    EXPECT_EQ(4, (int)result.size());
}

INSTANTIATE_TEST_CASE_P(Imgproc, ConvexityDefects_regression_5908,
        testing::Combine(
                testing::Bool(),
                testing::Values(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        ));

TEST(Imgproc_FitLine, regression_15083)
{
    int points2i_[] = {
        432, 654,
        370, 656,
        390, 656,
        410, 656,
        348, 658
    };
    Mat points(5, 1, CV_32SC2, points2i_);

    Vec4f lineParam;
    fitLine(points, lineParam, DIST_L1, 0, 0.01, 0.01);
    EXPECT_GE(fabs(lineParam[0]), fabs(lineParam[1]) * 4) << lineParam;
}

TEST(Imgproc_FitLine, regression_4903)
{
    float points2f_[] = {
        1224.0, 576.0,
        1234.0, 683.0,
        1215.0, 471.0,
        1184.0, 137.0,
        1079.0, 377.0,
        1239.0, 788.0,
    };
    Mat points(6, 1, CV_32FC2, points2f_);

    Vec4f lineParam;
    fitLine(points, lineParam, DIST_WELSCH, 0, 0.01, 0.01);
    EXPECT_GE(fabs(lineParam[1]), fabs(lineParam[0]) * 4) << lineParam;
}

#if 0
#define DRAW(x) x
#else
#define DRAW(x)
#endif

// the Python test by @hannarud is converted to C++; see the issue #4539
TEST(Imgproc_ConvexityDefects, ordering_4539)
{
    int contour[][2] =
    {
        {26,  9}, {25, 10}, {24, 10}, {23, 10}, {22, 10}, {21, 10}, {20, 11}, {19, 11}, {18, 11}, {17, 12},
        {17, 13}, {18, 14}, {18, 15}, {18, 16}, {18, 17}, {19, 18}, {19, 19}, {20, 20}, {21, 21}, {21, 22},
        {22, 23}, {22, 24}, {23, 25}, {23, 26}, {24, 27}, {25, 28}, {26, 29}, {27, 30}, {27, 31}, {28, 32},
        {29, 32}, {30, 33}, {31, 34}, {30, 35}, {29, 35}, {30, 35}, {31, 34}, {32, 34}, {33, 34}, {34, 33},
        {35, 32}, {35, 31}, {35, 30}, {36, 29}, {37, 28}, {37, 27}, {38, 26}, {39, 25}, {40, 24}, {40, 23},
        {41, 22}, {42, 21}, {42, 20}, {42, 19}, {43, 18}, {43, 17}, {44, 16}, {45, 15}, {45, 14}, {46, 13},
        {46, 12}, {45, 11}, {44, 11}, {43, 11}, {42, 10}, {41, 10}, {40,  9}, {39,  9}, {38,  9}, {37,  9},
        {36,  9}, {35,  9}, {34,  9}, {33,  9}, {32,  9}, {31,  9}, {30,  9}, {29,  9}, {28,  9}, {27,  9}
    };
    int npoints = (int)(sizeof(contour)/sizeof(contour[0][0])/2);
    Mat contour_(1, npoints, CV_32SC2, contour);
    vector<Point> hull;
    vector<int> hull_ind;
    vector<Vec4i> defects;

    // first, check the original contour as-is, without intermediate fillPoly/drawContours.
    convexHull(contour_, hull_ind, false, false);
    EXPECT_THROW( convexityDefects(contour_, hull_ind, defects), cv::Exception );

    int scale = 20;
    contour_ *= (double)scale;

    Mat canvas_gray(Size(60*scale, 45*scale), CV_8U, Scalar::all(0));
    const Point* ptptr = contour_.ptr<Point>();
    fillPoly(canvas_gray, &ptptr, &npoints, 1, Scalar(255, 255, 255));

    vector<vector<Point> > contours;
    findContours(canvas_gray, contours, noArray(), RETR_LIST, CHAIN_APPROX_SIMPLE);
    convexHull(contours[0], hull_ind, false, false);

    // the original contour contains self-intersections,
    // therefore convexHull does not return a monotonous sequence of points
    // and therefore convexityDefects throws an exception
    EXPECT_THROW( convexityDefects(contours[0], hull_ind, defects), cv::Exception );

#if 1
    // one way to eliminate the contour self-intersection in this particular case is to apply dilate(),
    // so that the self-repeating points are not self-repeating anymore
    dilate(canvas_gray, canvas_gray, Mat());
#else
    // another popular technique to eliminate such thin "hair" is to use morphological "close" operation,
    // which is erode() + dilate()
    erode(canvas_gray, canvas_gray, Mat());
    dilate(canvas_gray, canvas_gray, Mat());
#endif

    // after the "fix", the newly retrieved contour should not have self-intersections,
    // and everything should work well
    findContours(canvas_gray, contours, noArray(), RETR_LIST, CHAIN_APPROX_SIMPLE);
    convexHull(contours[0], hull, false, true);
    convexHull(contours[0], hull_ind, false, false);

    DRAW(Mat canvas(Size(60*scale, 45*scale), CV_8UC3, Scalar::all(0));
        drawContours(canvas, contours, -1, Scalar(255, 255, 255), -1));

    size_t nhull = hull.size();
    ASSERT_EQ( nhull, hull_ind.size() );

    if( nhull > 2 )
    {
        bool initial_lt = hull_ind[0] < hull_ind[1];
        for( size_t i = 0; i < nhull; i++ )
        {
            int ind = hull_ind[i];
            Point pt = contours[0][ind];

            ASSERT_EQ(pt, hull[i]);
            if( i > 0 )
            {
                // check that the convex hull indices are monotone
                if( initial_lt )
                {
                    ASSERT_LT(hull_ind[i-1], hull_ind[i]);
                }
                else
                {
                    ASSERT_GT(hull_ind[i-1], hull_ind[i]);
                }
            }
            DRAW(circle(canvas, pt, 7, Scalar(180, 0, 180), -1, LINE_AA);
                putText(canvas, format("%d (%d)", (int)i, ind), pt+Point(15, 0), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 0, 200), 1, LINE_AA));
            //printf("%d. ind=%d, pt=(%d, %d)\n", (int)i, ind, pt.x, pt.y);
        }
    }

    convexityDefects(contours[0], hull_ind, defects);

    for(size_t i = 0; i < defects.size(); i++ )
    {
        Vec4i d = defects[i];
        //printf("defect %d. start=%d, end=%d, farthest=%d, depth=%d\n", (int)i, d[0], d[1], d[2], d[3]);
        EXPECT_LT(d[0], d[1]);
        EXPECT_LE(d[0], d[2]);
        EXPECT_LE(d[2], d[1]);

        DRAW(Point start = contours[0][d[0]];
             Point end = contours[0][d[1]];
             Point far = contours[0][d[2]];
             line(canvas, start, end, Scalar(255, 255, 128), 3, LINE_AA);
             line(canvas, start, far, Scalar(255, 150, 255), 3, LINE_AA);
             line(canvas, end, far, Scalar(255, 150, 255), 3, LINE_AA);
             circle(canvas, start, 7, Scalar(0, 0, 255), -1, LINE_AA);
             circle(canvas, end, 7, Scalar(0, 0, 255), -1, LINE_AA);
             circle(canvas, far, 7, Scalar(255, 0, 0), -1, LINE_AA));
    }

    DRAW(imshow("defects", canvas);
         waitKey());
}

#undef DRAW

TEST(Imgproc_ConvexHull, overflow)
{
    std::vector<Point> points;
    std::vector<Point2f> pointsf;

    points.push_back(Point(14763, 2890));
    points.push_back(Point(14388, 72088));
    points.push_back(Point(62810, 72274));
    points.push_back(Point(63166, 3945));
    points.push_back(Point(56782, 3945));
    points.push_back(Point(56763, 3077));
    points.push_back(Point(34666, 2965));
    points.push_back(Point(34547, 2953));
    points.push_back(Point(34508, 2866));
    points.push_back(Point(34429, 2965));

    size_t i, n = points.size();
    for( i = 0; i < n; i++ )
        pointsf.push_back(Point2f(points[i]));

    std::vector<int> hull;
    std::vector<int> hullf;

    convexHull(points, hull, false, false);
    convexHull(pointsf, hullf, false, false);

    ASSERT_EQ(hull, hullf);
}

static
bool checkMinAreaRect(const RotatedRect& rr, const Mat& c, double eps = 0.5f)
{
    int N = c.rows;

    Mat rr_pts;
    boxPoints(rr, rr_pts);

    double maxError = 0.0;
    int nfailed = 0;
    for (int i = 0; i < N; i++)
    {
        double d = pointPolygonTest(rr_pts, c.at<Point2f>(i), true);
        maxError = std::max(-d, maxError);
        if (d < -eps)
            nfailed++;
    }

    if (nfailed)
        std::cout << "nfailed=" << nfailed << " (total=" << N << ")   maxError=" << maxError << std::endl;
    return nfailed == 0;
}

TEST(Imgproc_minAreaRect, reproducer_18157)
{
    const int N = 168;
    float pts_[N][2] = {
        { 1903, 266 }, { 1897, 267 }, { 1893, 268 }, { 1890, 269 },
        { 1878, 275 }, { 1875, 277 }, { 1872, 279 }, { 1868, 282 },
        { 1862, 287 }, { 1750, 400 }, { 1748, 402 }, { 1742, 407 },
        { 1742, 408 }, { 1740, 410 }, { 1738, 412 }, { 1593, 558 },
        { 1590, 560 }, { 1588, 562 }, { 1586, 564 }, { 1580, 570 },
        { 1443, 709 }, { 1437, 714 }, { 1435, 716 }, { 1304, 848 },
        { 1302, 850 }, { 1292, 860 }, { 1175, 979 }, { 1172, 981 },
        { 1049, 1105 }, { 936, 1220 }, { 933, 1222 }, { 931, 1224 },
        { 830, 1326 }, { 774, 1383 }, { 769, 1389 }, { 766, 1393 },
        { 764, 1396 }, { 762, 1399 }, { 760, 1402 }, { 757, 1408 },
        { 757, 1410 }, { 755, 1413 }, { 754, 1416 }, { 753, 1420 },
        { 752, 1424 }, { 752, 1442 }, { 753, 1447 }, { 754, 1451 },
        { 755, 1454 }, { 757, 1457 }, { 757, 1459 }, { 761, 1467 },
        { 763, 1470 }, { 765, 1473 }, { 767, 1476 }, { 771, 1481 },
        { 779, 1490 }, { 798, 1510 }, { 843, 1556 }, { 847, 1560 },
        { 851, 1564 }, { 863, 1575 }, { 907, 1620 }, { 909, 1622 },
        { 913, 1626 }, { 1154, 1866 }, { 1156, 1868 }, { 1158, 1870 },
        { 1207, 1918 }, { 1238, 1948 }, { 1252, 1961 }, { 1260, 1968 },
        { 1264, 1971 }, { 1268, 1974 }, { 1271, 1975 }, { 1273, 1977 },
        { 1283, 1982 }, { 1286, 1983 }, { 1289, 1984 }, { 1294, 1985 },
        { 1300, 1986 }, { 1310, 1986 }, { 1316, 1985 }, { 1320, 1984 },
        { 1323, 1983 }, { 1326, 1982 }, { 1338, 1976 }, { 1341, 1974 },
        { 1344, 1972 }, { 1349, 1968 }, { 1358, 1960 }, { 1406, 1911 },
        { 1421, 1897 }, { 1624, 1693 }, { 1788, 1528 }, { 1790, 1526 },
        { 1792, 1524 }, { 1794, 1522 }, { 1796, 1520 }, { 1798, 1518 },
        { 1800, 1516 }, { 1919, 1396 }, { 1921, 1394 }, { 2038, 1275 },
        { 2047, 1267 }, { 2048, 1265 }, { 2145, 1168 }, { 2148, 1165 },
        { 2260, 1052 }, { 2359, 952 }, { 2434, 876 }, { 2446, 863 },
        { 2450, 858 }, { 2453, 854 }, { 2455, 851 }, { 2457, 846 },
        { 2459, 844 }, { 2460, 842 }, { 2460, 840 }, { 2462, 837 },
        { 2463, 834 }, { 2464, 830 }, { 2465, 825 }, { 2465, 809 },
        { 2464, 804 }, { 2463, 800 }, { 2462, 797 }, { 2461, 794 },
        { 2456, 784 }, { 2454, 781 }, { 2452, 778 }, { 2450, 775 },
        { 2446, 770 }, { 2437, 760 }, { 2412, 734 }, { 2410, 732 },
        { 2408, 730 }, { 2382, 704 }, { 2380, 702 }, { 2378, 700 },
        { 2376, 698 }, { 2372, 694 }, { 2370, 692 }, { 2368, 690 },
        { 2366, 688 }, { 2362, 684 }, { 2360, 682 }, { 2252, 576 },
        { 2250, 573 }, { 2168, 492 }, { 2166, 490 }, { 2085, 410 },
        { 2026, 352 }, { 1988, 315 }, { 1968, 296 }, { 1958, 287 },
        { 1953, 283 }, { 1949, 280 }, { 1946, 278 }, { 1943, 276 },
        { 1940, 274 }, { 1936, 272 }, { 1934, 272 }, { 1931, 270 },
        { 1928, 269 }, { 1925, 268 }, { 1921, 267 }, { 1915, 266 }
    };

    Mat contour(N, 1, CV_32FC2, (void*)pts_);

    RotatedRect rr = cv::minAreaRect(contour);

    EXPECT_TRUE(checkMinAreaRect(rr, contour)) << rr.center << " " << rr.size << " " << rr.angle;
}

TEST(Imgproc_minAreaRect, reproducer_19769_lightweight)
{
    const int N = 23;
    float pts_[N][2] = {
            {1325, 732}, {1248, 808}, {582, 1510}, {586, 1524},
            {595, 1541}, {599, 1547}, {789, 1745}, {829, 1786},
            {997, 1958}, {1116, 2074}, {1207, 2066}, {1216, 2058},
            {1231, 2044}, {1265, 2011}, {2036, 1254}, {2100, 1191},
            {2169, 1123}, {2315, 979}, {2395, 900}, {2438, 787},
            {2434, 782}, {2416, 762}, {2266, 610}
    };
    Mat contour(N, 1, CV_32FC2, (void*)pts_);

    RotatedRect rr = cv::minAreaRect(contour);

    EXPECT_TRUE(checkMinAreaRect(rr, contour)) << rr.center << " " << rr.size << " " << rr.angle;
}

TEST(Imgproc_minAreaRect, reproducer_19769)
{
    const int N = 169;
    float pts_[N][2] = {
            {1854, 227}, {1850, 228}, {1847, 229}, {1835, 235},
            {1832, 237}, {1829, 239}, {1825, 242}, {1818, 248},
            {1807, 258}, {1759, 306}, {1712, 351}, {1708, 356},
            {1658, 404}, {1655, 408}, {1602, 459}, {1599, 463},
            {1542, 518}, {1477, 582}, {1402, 656}, {1325, 732},
            {1248, 808}, {1161, 894}, {1157, 898}, {1155, 900},
            {1068, 986}, {1060, 995}, {1058, 997}, {957, 1097},
            {956, 1097}, {814, 1238}, {810, 1242}, {805, 1248},
            {610, 1442}, {603, 1450}, {599, 1455}, {596, 1459},
            {594, 1462}, {592, 1465}, {590, 1470}, {588, 1472},
            {586, 1476}, {586, 1478}, {584, 1481}, {583, 1485},
            {582, 1490}, {582, 1510}, {583, 1515}, {584, 1518},
            {585, 1521}, {586, 1524}, {593, 1538}, {595, 1541},
            {597, 1544}, {599, 1547}, {603, 1552}, {609, 1559},
            {623, 1574}, {645, 1597}, {677, 1630}, {713, 1667},
            {753, 1707}, {789, 1744}, {789, 1745}, {829, 1786},
            {871, 1828}, {909, 1867}, {909, 1868}, {950, 1910},
            {953, 1912}, {997, 1958}, {1047, 2009}, {1094, 2056},
            {1105, 2066}, {1110, 2070}, {1113, 2072}, {1116, 2074},
            {1119, 2076}, {1122, 2077}, {1124, 2079}, {1130, 2082},
            {1133, 2083}, {1136, 2084}, {1139, 2085}, {1142, 2086},
            {1148, 2087}, {1166, 2087}, {1170, 2086}, {1174, 2085},
            {1177, 2084}, {1180, 2083}, {1188, 2079}, {1190, 2077},
            {1193, 2076}, {1196, 2074}, {1199, 2072}, {1202, 2070},
            {1207, 2066}, {1216, 2058}, {1231, 2044}, {1265, 2011},
            {1314, 1962}, {1360, 1917}, {1361, 1917}, {1408, 1871},
            {1457, 1822}, {1508, 1773}, {1512, 1768}, {1560, 1722},
            {1617, 1665}, {1671, 1613}, {1730, 1554}, {1784, 1502},
            {1786, 1500}, {1787, 1498}, {1846, 1440}, {1850, 1437},
            {1908, 1380}, {1974, 1314}, {2034, 1256}, {2036, 1254},
            {2100, 1191}, {2169, 1123}, {2242, 1051}, {2315, 979},
            {2395, 900}, {2426, 869}, {2435, 859}, {2438, 855},
            {2440, 852}, {2442, 849}, {2443, 846}, {2445, 844},
            {2446, 842}, {2446, 840}, {2448, 837}, {2449, 834},
            {2450, 829}, {2450, 814}, {2449, 809}, {2448, 806},
            {2447, 803}, {2442, 793}, {2440, 790}, {2438, 787},
            {2434, 782}, {2428, 775}, {2416, 762}, {2411, 758},
            {2342, 688}, {2340, 686}, {2338, 684}, {2266, 610},
            {2260, 605}, {2170, 513}, {2075, 417}, {2073, 415},
            {2069, 412}, {1955, 297}, {1955, 296}, {1913, 254},
            {1904, 246}, {1897, 240}, {1894, 238}, {1891, 236},
            {1888, 234}, {1880, 230}, {1877, 229}, {1874, 228},
            {1870, 227}
    };
    Mat contour(N, 1, CV_32FC2, (void*)pts_);

    RotatedRect rr = cv::minAreaRect(contour);

    EXPECT_TRUE(checkMinAreaRect(rr, contour)) << rr.center << " " << rr.size << " " << rr.angle;
}

TEST(Imgproc_minEnclosingTriangle, regression_17585)
{
    const int N = 3;
    float pts_[N][2] = { {0, 0}, {0, 1}, {1, 1} };
    cv::Mat points(N, 2, CV_32FC1, static_cast<void*>(pts_));
    vector<Point2f> triangle;

    EXPECT_NO_THROW(minEnclosingTriangle(points, triangle));
}

TEST(Imgproc_minEnclosingTriangle, regression_20890)
{
    vector<Point> points;
    points.push_back(Point(0, 0));
    points.push_back(Point(0, 1));
    points.push_back(Point(1, 1));
    vector<Point2f> triangle;

    EXPECT_NO_THROW(minEnclosingTriangle(points, triangle));
}

TEST(Imgproc_minEnclosingTriangle, regression_mat_with_diff_channels)
{
    const int N = 3;
    float pts_[N][2] = { {0, 0}, {0, 1}, {1, 1} };
    cv::Mat points1xN(1, N, CV_32FC2, static_cast<void*>(pts_));
    cv::Mat pointsNx1(N, 1, CV_32FC2, static_cast<void*>(pts_));
    vector<Point2f> triangle;

    EXPECT_NO_THROW(minEnclosingTriangle(points1xN, triangle));
    EXPECT_NO_THROW(minEnclosingTriangle(pointsNx1, triangle));
}

}} // namespace
/* End of file. */
