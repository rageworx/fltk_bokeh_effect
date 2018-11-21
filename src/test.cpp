#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#endif

#include <unistd.h>

#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>

#if defined(__linux__)
#include <png.h>
#else
#include <FL/images/png.h>
#endif

#include <string>

#include "libbokeh.h"
#include "fl_imgtk.h"
#include "tick.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////

#define APP_VERSION_STR     "0.1.8.36"

////////////////////////////////////////////////////////////////////////////////

bool save2png( Fl_RGB_Image* imgcached, const char* fpath );

bool convImage2RGB( Fl_RGB_Image* src, Fl_RGB_Image* &dst )
{
    if ( src != NULL )
    {
        unsigned img_w = src->w();
        unsigned img_h = src->h();
        unsigned img_d = src->d();
        unsigned imgsz = img_w * img_h;

        uchar* cdata = NULL;

        switch( img_d )
        {
            case 1: /// single gray
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz * 3 ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            cdata[ cnt*3 + 0 ] = pdata[ cnt ];
                            cdata[ cnt*3 + 1 ] = pdata[ cnt ];
                            cdata[ cnt*3 + 2 ] = pdata[ cnt ];
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 3 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;

            case 2: /// Must be RGB565
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz * 3 ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            cdata[ cnt*3 + 0 ] = ( pdata[ cnt ] & 0xF800 ) >> 11;
                            cdata[ cnt*3 + 1 ] = ( pdata[ cnt ] & 0x07E0 ) >> 5;
                            cdata[ cnt*3 + 2 ] = ( pdata[ cnt ] & 0x001F );
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 3 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;
                
            case 4: /// removing alpha ...
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz * 3 ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            float alp = (float)( pdata[ cnt * 4 + 3 ] ) / 255.f;
                            cdata[ cnt*3 + 0 ] = pdata[ cnt * 4 + 0 ] * alp;
                            cdata[ cnt*3 + 1 ] = pdata[ cnt * 4 + 1 ] * alp;
                            cdata[ cnt*3 + 2 ] = pdata[ cnt * 4 + 2 ] * alp;
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 3 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;          

            default:
                {
                    dst = (Fl_RGB_Image*)src->copy();

                    if ( dst != NULL )
                    {
                        return true;
                    }
                }
                break;
        }
    }

    return false;
}

bool convImage2Mono( Fl_RGB_Image* src, Fl_RGB_Image* &dst )
{
    if ( src != NULL )
    {
        unsigned img_w = src->w();
        unsigned img_h = src->h();
        unsigned img_d = src->d();
        unsigned imgsz = img_w * img_h;

        uchar* cdata = NULL;

        switch( img_d )
        {
			default:
            case 1: /// single gray
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
							cdata[ cnt ] = pdata[ cnt ];
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 1 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;

            case 2: /// Must be RGB565
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            unsigned _s = 0;
                            _s += ( pdata[ cnt ] & 0xF800 ) >> 11;
                            _s += ( pdata[ cnt ] & 0x07E0 ) >> 5;
                            _s += ( pdata[ cnt ] & 0x001F );
							cdata[ cnt ] = (uchar) ( _s / 3 );
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 1 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;
                
            case 3:
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            unsigned _s = 0;
                            _s += pdata[ (cnt * 3) + 0 ];
                            _s += pdata[ (cnt * 3) + 1 ];
                            _s += pdata[ (cnt * 3) + 2 ];
							
							cdata[ cnt ] = (uchar)( _s / 3 );
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 1 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;          

           case 4: /// removing alpha ...
                {
                    const uchar* pdata = (const uchar*)src->data()[0];
                    cdata = new uchar[ imgsz ];
                    if ( cdata != NULL )
                    {
                        #pragma omp parallel for
                        for( unsigned cnt=0; cnt<imgsz; cnt++ )
                        {
                            float alp = (float)( pdata[ cnt * 4 + 3 ] ) / 255.f;
                            unsigned _s = 0;
                            _s += pdata[ (cnt * 4) + 0 ] * alp;
                            _s += pdata[ (cnt * 4) + 1 ] * alp;
                            _s += pdata[ (cnt * 4) + 2 ] * alp;
							
							cdata[ cnt ] = (uchar) ( _s / 3 );
                        }

                        dst = new Fl_RGB_Image( cdata, img_w, img_h, 1 );

                        if ( dst != NULL )
                        {
                            return true;
                        }
                    }
                }
                break;          

        }
    }

    return false;
}


int testImageFile( const char* imgfp, uchar** buff,size_t* buffsz )
{
    int reti = -1;

    if ( imgfp != NULL )
    {
        FILE* fp = fopen( imgfp, "rb" );
        if ( fp != NULL )
        {
            fseek( fp, 0L, SEEK_END );
            size_t flen = ftell( fp );
            fseek( fp, 0L, SEEK_SET );

            if ( flen > 32 )
            {
                // Test
                char testbuff[32] = {0,};

                fread( testbuff, 1, 32, fp );
                fseek( fp, 0, SEEK_SET );

                const uchar jpghdr[3] = { 0xFF, 0xD8, 0xFF };

                // is JPEG ???
                if( strncmp( &testbuff[0], (const char*)jpghdr, 3 ) == 0 )
                {
                    reti = 1; /// JPEG.
                }
                else
                if( strncmp( &testbuff[1], "PNG", 3 ) == 0 )
                {
                    reti = 2; /// PNG.
                }
                else
                if( strncmp( &testbuff[0], "BM", 2 ) == 0 )
                {
                    reti = 3; /// BMP.
                }

                if ( reti > 0 )
                {
                    *buff = new uchar[ flen ];
                    if ( *buff != NULL )
                    {
                        fread( *buff, 1, flen, fp );

                        if( buffsz != NULL )
                        {
                            *buffsz = flen;
                        }
                    }
                }
            }

            fclose( fp );
        }
    }

    return reti;
}

bool save2png( Fl_RGB_Image* imgcached, const char* fpath )
{
    if ( imgcached == NULL )
        return false;

	int img_d      = imgcached->d();
	int png_s_type = PNG_COLOR_TYPE_RGB;
	
	switch( img_d )
	{	
		case 1: 
			png_s_type = PNG_COLOR_TYPE_GRAY;
            break;
		
		case 3:
			png_s_type = PNG_COLOR_TYPE_RGB;
            break;
			
		case 4:
			png_s_type = PNG_COLOR_TYPE_RGBA;
            break;
	}
	
    FILE* fp = fopen( fpath, "wb" );
    if ( fp == NULL )
        return false;

    png_structp png_ptr     = NULL;
    png_infop   info_ptr    = NULL;
    png_bytep   row         = NULL;

    png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    if ( png_ptr != NULL )
    {
        info_ptr = png_create_info_struct( png_ptr );
        if ( info_ptr != NULL )
        {
            if ( setjmp( png_jmpbuf( (png_ptr) ) ) == 0 )
            {
                int mx = imgcached->w();
                int my = imgcached->h();

                png_init_io( png_ptr, fp );
                png_set_IHDR( png_ptr,
                              info_ptr,
                              mx,
                              my,
                              8,
                              png_s_type,
                              PNG_INTERLACE_NONE,
                              PNG_COMPRESSION_TYPE_BASE,
                              PNG_FILTER_TYPE_BASE);

                png_write_info( png_ptr, info_ptr );

                row = (png_bytep)malloc( mx * img_d * sizeof(png_bytep) );
				
                if ( row != NULL )
                {
                    const char* buf = imgcached->data()[0];
                    int bque = 0;

                    for( int y=0; y<my; y++ )
                    {
                        for( int x=0; x<mx; x++ )
                        {
							for( int d=0; d<img_d; d++ )
                            {
								row[ (x*img_d)+d ] = buf[ bque+d ];
							}
							
                            bque += img_d;
                        }

                        png_write_row( png_ptr, row );
                    }
					
                    png_write_end( png_ptr, NULL );

                    fclose( fp );

                    free(row);
                }

                png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
                png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

                return true;
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static string   path_me;
static string   file_me;
static string   file_src;
static string   file_bokeh;
static string   file_dst;
static string   file_cov;

bool parseArgs( int argc, char** argv )
{
    for( int cnt=0; cnt<argc; cnt++ )
    {
        string strtmp = argv[ cnt ];
        size_t fpos   = string::npos;

        if ( cnt == 0 )
        {
            fpos = strtmp.find_last_of( "\\" );

            if ( fpos == string::npos )
            {
                fpos = strtmp.find_last_of( "/" );
            }

            if ( fpos != string::npos )
            {
                path_me = strtmp.substr( 0, fpos );
                file_me = strtmp.substr( fpos + 1 );
            }
            else
            {
                file_me = strtmp;
            }
        }
        else
        {
            if ( file_src.size() == 0 )
            {
                file_src = strtmp;
            }
            else
			if ( file_bokeh.size() == 0 )
			{
				file_bokeh = strtmp;
			}
			else
            if ( file_dst.size() == 0 )
            {
                file_dst = strtmp;
            }
        }
    }
    
    if ( ( file_src.size() > 0 ) && ( file_bokeh.size() > 0 ) 
		  && ( file_dst.size() == 0 ) )
    {
        string convname = file_src;
        string srcext;
        
        // changes name without file extention.
        size_t posdot = file_src.find_last_of( "." );
        if ( posdot != string::npos )
        {
            convname = file_src.substr( 0, posdot );
            srcext   = file_src.substr( posdot );
        }
        
        convname += "_bokeh";
        if ( srcext.size() > 0 )
        {
            if ( ( srcext != ".png" ) || ( srcext != ".PNG" ) )
            {
                convname += ".png";
            }
            else
            {
                convname += srcext;
            }
        }
        
        file_dst = convname;
    }
    
    if ( ( file_src.size() > 0 ) && ( file_dst.size() > 0 ) )
    {
        string convname = file_src;
        string srcext;
        
        // changes name without file extention.
        size_t posdot = file_src.find_last_of( "." );
        if ( posdot != string::npos )
        {
            convname = file_src.substr( 0, posdot );
            srcext   = file_src.substr( posdot );
        }
        
        convname += "_convolution";
        if ( srcext.size() > 0 )
        {
            if ( ( srcext != ".png" ) || ( srcext != ".PNG" ) )
            {
                convname += ".png";
            }
            else
            {
                convname += srcext;
            }
        }
        
        file_cov = convname;

        return true;
    }
    
    return false;
}

void printAbout()
{
    printf( "%s : libbokeh testing program with FLTK-1.3.4-2-ts, ver %s\n", 
            file_me.c_str(),
            APP_VERSION_STR );
    printf( "(C)Copyrighted ~2018 Raphael Kim\n\n" );
    fflush( stdout );   
}

void printUsage()
{
    printf( "  usage:\n" );
    printf( "      %s [source image file] [bokeh file] (output image file)\n", 
            file_me.c_str() );
    printf( "\n" );
}

Fl_RGB_Image* loadImg( string fname )
{
    printf( "- Loading images : %s -> ", fname.c_str() );

    uchar* imgbuff = NULL;
    size_t imgsz = 0;
		
    int imgtype = testImageFile( fname.c_str(), &imgbuff, &imgsz );
    if ( imgtype > 0 )
    {        
		Fl_RGB_Image* imgTest = NULL;
		
        switch( imgtype )
        {
            case 1: /// JPEG
                printf( "JPEG | ");
                
                imgTest = new Fl_JPEG_Image( "JPGIMG",
                                            (const uchar*)imgbuff );
                break;

            case 2: /// PNG
                printf( "PNG | " );
                
                imgTest = new Fl_PNG_Image( "PNGIMAGE",
                                           (const uchar*)imgbuff, imgsz );
                break;

            case 3: /// BMP
                printf( "BMP | " );
                imgTest = fl_imgtk::createBMPmemory( (const char*)imgbuff, imgsz );
                break;
                
            default: /// unknown...
                printf( "Unsupported !\n" );
                break;
        }
        
        if ( imgTest != NULL )
        {
            printf( "%ux%ux%u bytes\n", imgTest->w(), imgTest->h(), imgTest->d() );
            fflush( stdout );
        }
        
        delete[] imgbuff;
        imgbuff = NULL;
        imgsz = 0;
		
		return imgTest;
    }
    else
    {
        printf( "Failed.\n" );
    }
	
	return NULL;
}

int main( int argc, char** argv )
{   
    if ( parseArgs( argc, argv ) == false )
    {
        printAbout();
        printUsage();
        return -1;
    }

    printAbout();
    
    Fl_RGB_Image* imgSrc   = loadImg( file_src );
	Fl_RGB_Image* imgBokeh = loadImg( file_bokeh );    
    
    if ( ( imgSrc != NULL ) && ( imgBokeh != NULL ) ) 
    {
        unsigned origin_w = imgSrc->w();
        unsigned origin_h = imgSrc->h();

        Fl_RGB_Image* imgRGB = NULL;
		Fl_RGB_Image* imgMask = NULL;
                
		printf( "- Converting common images ... " );

        unsigned mask_w = imgBokeh->w();
        unsigned mask_h = imgBokeh->h();        
        unsigned expand_sz_w = origin_w + ( mask_w * 2 );
        unsigned expand_sz_h = origin_h + ( mask_h * 2 );

        // Expand soruce image -
        Fl_RGB_Image* imgTmpSrc = imgSrc;
        imgSrc = fl_imgtk::rescale( imgTmpSrc,
                                    expand_sz_w,
                                    expand_sz_h,
                                    fl_imgtk::BILINEAR );
        //fl_imgtk::brightness_ex( imgSrc, -50 );
        fl_imgtk::drawonimage( imgSrc, 
                               imgTmpSrc,
                               ( expand_sz_w - origin_w ) / 2,
                               ( expand_sz_h - origin_h ) / 2 );

        fl_imgtk::discard_user_rgb_image( imgTmpSrc );

        convImage2RGB( imgSrc, imgRGB );
        
        if ( ( imgBokeh->w() <= imgSrc->w() ) 
              && ( imgBokeh->h() <= imgSrc->h() ) )
        {
            /*
            Fl_RGB_Image* imgTmp = imgBokeh;
            imgBokeh= fl_imgtk::makeanempty( imgSrc->w(), imgSrc->h(), 3, 0x0 );
            int put_x = 0;
            int put_y = imgSrc->h() - imgTmp->h();
            fl_imgtk::drawonimage( imgBokeh, imgTmp, put_x, put_y );
            
            fl_imgtk::discard_user_rgb_image( imgTmp );
            */
        }
        else
        {
            // Error !
            // Bokeh must be smaller than source image.
            printf( "Error.\n" );
            printf( "-> Bokeh image is larger than source image.\n" );
        }
        
		convImage2Mono( imgBokeh, imgMask );
		printf( "Ok.\n" );
		fflush( stdout );
        
        fl_imgtk::discard_user_rgb_image( imgSrc );
		fl_imgtk::discard_user_rgb_image( imgBokeh );
        
        if ( ( imgRGB->w() > 0 ) && ( imgRGB->h() > 0 ) && ( imgRGB->d() >= 3 ) )
        {
            const uchar* refbuff = (const uchar*)imgRGB->data()[0];
            unsigned     ref_w   = imgRGB->w();
            unsigned     ref_h   = imgRGB->h();
            unsigned     ref_d   = imgRGB->d();
			const uchar* refmbuf = (const uchar*)imgMask->data()[0];
			
            uchar*       outbuff = NULL;
            unsigned     outsz   = 0;

			printf( "- Processing bokeh effect ... " );
			fflush( stdout );
	    
            unsigned perf0   = tick::getTickCount();
 		
            /*
			bool retb = ProcessBokeh( refbuff,
			                          ref_w, ref_h, ref_d,
									  refmbuf,
									  outbuff );
            */
            bool retb = ProcessFastBokeh( refbuff,
                                          ref_w, ref_h, ref_d,
                                          refmbuf,
                                          mask_w, mask_h,
                                          outbuff );
									  
	        unsigned perf1    = tick::getTickCount();

            printf( "done ( %d ) in %u ms.\n", 
                    (int)retb, perf1 - perf0 );
			fflush( stdout );


			if ( retb == true )
			{
                Fl_RGB_Image* imgWrite = NULL;
				Fl_RGB_Image* imgWriteSrc = new Fl_RGB_Image( outbuff, 
                                                              ref_w, 
                                                              ref_h, 
                                                              3 );
                if ( imgWriteSrc != NULL )
                {
                    unsigned crop_l = mask_w + ( mask_w * 0.55f );
                    unsigned crop_t = mask_h * 0.55f;
                    // Crop image to origin size.
                    imgWrite = fl_imgtk::crop( imgWriteSrc,
                                               crop_l,
                                               crop_t,
                                               origin_w,
                                               origin_h );


                    fl_imgtk::discard_user_rgb_image( imgWriteSrc );
                }

				if ( imgWrite != NULL )
				{
					printf( "- Writing : %s ... ", file_dst.c_str() );
					fflush(stdout);
					
					save2png( imgWrite, file_dst.c_str() );

                    printf( "Done.\n" );
                    fflush( stdout );
					
					delete imgWrite;
				}
			}
			
            delete imgRGB;
			delete imgMask;			
        }
        else
        {
            printf( "- Error: Unsupported image.\n" );
        }
    }
    else
    {
        printf( "- Failed to load image.\n" );
    }

    return 0;
}

