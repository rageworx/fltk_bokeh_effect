// Code from : 
// https://www.scratchapixel.com/code.php?id=51&origin=/lessons/digital-imaging/simple-image-manipulations
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <cassert>
#include <exception>
#include <vector>

#include <omp.h>

#ifndef nullptr
    #define nullptr     NULL
#endif

using namespace std;

class Image
{
    public:
        
        struct RGBf
        {
            RGBf() : r(0), g(0), b(0)
            {
            }
            
            RGBf(float c) : r(c), g(c), b(c) 
            {
            }
            
            RGBf(float _r, float _g, float _b) : r(_r), g(_g), b(_b) 
            {
            }
            
            bool operator != (const RGBf &c) const 
            { 
                return c.r != r && c.g != g && c.b != b; 
            }
            
            RGBf& operator *= (const RGBf &RGBf) 
            { 
                r *= RGBf.r;
                g *= RGBf.g;
                b *= RGBf.b; 
                
                return *this; 
            }
            
            RGBf& operator += (const RGBf &RGBf) 
            { 
                r += RGBf.r;
                g += RGBf.g;
                b += RGBf.b; 
                
                return *this; 
            }
            
            friend float& operator += (float &f, const RGBf RGBf)
            { 
                f += (RGBf.r + RGBf.g + RGBf.b) / 3.f; 
                
                return f; 
            }
            
            float r, g, b;
        };
        
        Image() 
        : w(0), h(0), pixels(nullptr)
        {
            /* empty image */ 
        }

        Image(const unsigned int &_w, const unsigned int &_h, const RGBf &c = RGBf(0) ) 
        : w(_w), h(_h), pixels(nullptr)
        {
            pixels = new RGBf[w * h];
            
            if ( pixels != nullptr )
            {
                #pragma omp parallel for
                for (int i = 0; i < w * h; ++i) 
                {
                    pixels[i] = c;
                }
            }
        }
        
        Image(const Image &img) 
        : w(img.w), h(img.h), pixels(nullptr)
        {
            pixels = new RGBf[w * h];
            
            if ( pixels != NULL )
            {
                memcpy(pixels, img.pixels, sizeof(RGBf) * w * h);
            }
        }
        
        // move constructor
#ifndef GCC5
        Image(Image &&img) 
        : w(0), h(0), pixels(nullptr)
        {
            w       = img.w;
            h       = img.h;
            pixels  = img.pixels;
            img.pixels = nullptr;
            img.w   = 0;
            img.h   = 0;
        }
#endif /// of GCC5

        // move assignment operator
#ifndef GCC5
        Image& operator = (Image &img)
        {
            if (this != &img) 
            {
                if (pixels != nullptr) 
                    delete [] pixels;
                
                w       = img.w;
                h       = img.h;
                pixels  = img.pixels;
                img.pixels = nullptr;
                img.w   = 0;
                img.h   = 0;
            }
            
            return *this;
        }
#endif /// of GCC5

        RGBf& operator () (const unsigned &x, const unsigned int &y) const
        {
            //assert(x < w && y < h);
            return pixels[ y * w + x ];
        }
        
        Image& operator *= (const RGBf &RGBf)
        {
            #pragma omp parallel for
            for (int i = 0; i < w * h; ++i)
            {
                pixels[i] *= RGBf;
            }
            
            return *this;
        }
        
        Image& operator += (const Image &img)
        {
            #pragma omp parallel for
            for (int i = 0; i < w * h; ++i) 
            {
                pixels[i] += img[i];
            }
            
            return *this;
        }
        
        Image& operator /= (const float &div)
        {
            float invDiv = 1 / div;
            
            #pragma omp parallel for
            for (int i = 0; i < w * h; ++i) 
            {
                pixels[i] *= invDiv;
            }
            
            return *this;
        }
        
        friend Image operator * (const RGBf &RGBf, const Image &img)
        {
            Image tmp(img); 
            tmp *= RGBf;
            return tmp;
        }
        
        Image operator * (const Image &img)
        {
            Image tmp(*this);
            
            // multiply pixels together
            #pragma omp parallel for
            for (int i = 0; i < w * h; ++i)
            {
                tmp[i] *= img[i];
            }
            
            return tmp;
        }
        
        static Image circshift(const Image &img, unsigned shftx, unsigned shfty )
        {
            Image tmp(img.w, img.h);
            
            unsigned w = img.w;
            unsigned h = img.h;
            unsigned x;
            unsigned y;
            
            #pragma omp parallel for private(x)
            for ( y = 0; y<h; y++ )
            {
                unsigned ymod = ( y + shfty ) % h;
              
                for ( x = 0; x < w; x++ ) 
                {
                    unsigned xmod = ( x + shftx ) % w;
                    tmp[ ymod * w + xmod ] = img[ y * w + x ];
                }    
            }
            
            return tmp;
        }
        
        const RGBf& operator [] (const unsigned int &i) const 
        { 
            return pixels[i]; 
        }
        
        RGBf& operator [] (const unsigned int &i) 
        { 
            return pixels[i]; 
        }
        
        ~Image() 
        { 
            if (pixels != nullptr) 
            {
                delete[] pixels; 
            }
        }

    public:
        unsigned w;
        unsigned h;
        RGBf*    pixels;
        
};

//////////////////////////////////////////////////

static float intensity = 0.9f;

//////////////////////////////////////////////////

Image loadFromMemory( const unsigned char* buff, unsigned w, unsigned h, unsigned d )
{   
    Image img;
    
    if ( ( buff == NULL ) || ( w == 0 ) || ( h == 0 ) || ( d == 0 ) )
        return img;
    
    img.w = w; 
    img.h = h;
    img.pixels = new Image::RGBf[w * h];

    if ( img.pixels == nullptr )
    {
        // It must be failed to allocate memory,
        // Caller must be check image width and height.
        img.w = 0;
        img.h = 0;
        return img; 
    }
    
    // read each pixel one by one and convert bytes to floats
    #pragma omp parallel for
    for ( unsigned cnt=0; cnt<w*h; cnt++ ) 
    {
        float    pix[3] = {0.f};
        unsigned pixque = cnt * d;
        
        switch( d )
        {
            case 1:
                pix[0] = (float)buff[ pixque ];
                pix[1] = pix[0];
                pix[2] = pix[0];
                break;
                
            case 3:
                pix[0] = (float)buff[ pixque + 0 ];
                pix[1] = (float)buff[ pixque + 1 ];
                pix[2] = (float)buff[ pixque + 2 ];
                break;
                
            case 4:
                {
                    float af = (float)buff[ pixque + 4 ] / 255.f;
                    
                    pix[0] = (float)buff[ pixque + 0 ] * af;
                    pix[1] = (float)buff[ pixque + 1 ] * af;
                    pix[2] = (float)buff[ pixque + 2 ] * af;
                }
                break;
        }
        
        img.pixels[cnt].r = pix[0] / 255.f;
        img.pixels[cnt].g = pix[1] / 255.f;
        img.pixels[cnt].b = pix[2] / 255.f;
        
        // Multiply by 3 when pixel value overs intesity.
        // advanced to color distornation.
        float mp_r = img.pixels[cnt].r;
        float mp_g = img.pixels[cnt].g;
        float mp_b = img.pixels[cnt].b;
        unsigned mp_s = 0;

        if ( img.pixels[cnt].r > intensity ) 
        {
            mp_r *= 3.f;
            mp_s ++;
        }

        if ( img.pixels[cnt].g > intensity )
        {
            mp_g *= 3.f;
            mp_s ++;
        }

        if ( img.pixels[cnt].b > intensity )
        {
            mp_b *= 3.f;
            mp_s ++;
        }

        if ( mp_s > 2 )
        {
            img.pixels[cnt].r = mp_r;
            img.pixels[cnt].g = mp_g;
            img.pixels[cnt].b = mp_b;
        }
    }

    return img;
}

bool ProcessBokeh( const unsigned char* srcptr, 
                   unsigned srcw, unsigned srch, unsigned srcd,
                   const unsigned char* bokeh,  
                   unsigned char* &outptr )
{
    Image srcf  = loadFromMemory( srcptr, srcw, srch, srcd );   
    Image maskf = loadFromMemory( bokeh, srcw, srch, 1 );
    Image outf( srcw, srch );

    float total = 0;
    Image::RGBf kBlack = Image::RGBf(0);
   
    unsigned x = 0;
    unsigned y = 0;

    for ( y=0; y<srch; y++ ) 
    {
        #pragma omp parallel for reduction(+:total) shared(outf)
        for ( x=0; x<srcw; x++ ) 
        {
            if ( maskf(x, y) != kBlack ) 
            {
                #pragma omp task
                outf  += maskf(x, y) * Image::circshift( srcf, x, y );
                total += maskf(x, y);
            }
        }
    }
    
    outf /= total;
        
    unsigned outsz = srcw * srch;
    outptr = new unsigned char[ outsz * 3 ];
    
    if ( outptr != NULL )
    {
        #pragma omp parallel for
        for( unsigned cnt=0; cnt<outsz; cnt++ )
        {
            unsigned char uc_rgb[3] = {0,0,0};
            
            uc_rgb[0] = min( 1.f, outf.pixels[cnt].r ) * 255.f;
            uc_rgb[1] = min( 1.f, outf.pixels[cnt].g ) * 255.f;
            uc_rgb[2] = min( 1.f, outf.pixels[cnt].b ) * 255.f;

            memcpy( &outptr[ cnt * 3 ], uc_rgb, 3 );
        }
        
        return true;
    }
    
    return false;
}

bool ProcessFastBokeh( const unsigned char* srcptr, 
                       unsigned srcw, unsigned srch, unsigned srcd,
                       const unsigned char* bokeh,  
                       unsigned bkw, unsigned bkh,
                       unsigned char* &outptr )
{
    // check mask size.
    if ( ( srcw < bkw ) || ( srch < bkh ) ) 
        return false;

    Image srcf  = loadFromMemory( srcptr, srcw, srch, srcd );   
    Image maskf = loadFromMemory( bokeh, bkw, bkh, 1 );
    Image outf( srcw, srch );

    float total = 0;
    Image::RGBf kBlack = Image::RGBf(0);
   
    unsigned x = 0;
    unsigned y = 0;
    unsigned msk_x = bkw;
    unsigned msk_y = srch - bkh;

    // Don't need to all size of image, just repeats for mask size.
    for( y=msk_y; y<srch; y++ )
    {
        #pragma omp parallel for reduction(+:total) shared(outf)
        for( x=0; x<msk_x; x++ )
        {
            unsigned mx = x;
            unsigned my = y - msk_y;

            if ( maskf(mx, my) != kBlack ) 
            {
                #pragma omp task
                outf  += maskf(mx, my) * Image::circshift( srcf, x, y );
                total += maskf(mx, my);
            }
        }
    }
    
    outf /= total;
        
    unsigned outsz = srcw * srch;
    outptr = new unsigned char[ outsz * 3 ];
    
    if ( outptr != NULL )
    {
        #pragma omp parallel for
        for( unsigned cnt=0; cnt<outsz; cnt++ )
        {
            unsigned char uc_rgb[3] = {0,0,0};
            
            uc_rgb[0] = min( 1.f, outf.pixels[cnt].r ) * 255.f;
            uc_rgb[1] = min( 1.f, outf.pixels[cnt].g ) * 255.f;
            uc_rgb[2] = min( 1.f, outf.pixels[cnt].b ) * 255.f;

            memcpy( &outptr[ cnt * 3 ], uc_rgb, 3 );
        }
        
        return true;
    }
    
    return false;
}
