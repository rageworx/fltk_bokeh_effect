#ifndef __LIBBOKEH_H__

bool ProcessBokeh( const unsigned char* srcptr, 
                   unsigned srcw, unsigned srch, unsigned srcd,
                   const unsigned char* bokeh,  
				   unsigned char* &outptr );

bool ProcessFastBokeh( const unsigned char* srcptr, 
                       unsigned srcw, unsigned srch, unsigned srcd,
                       const unsigned char* bokeh,  
                       unsigned bkw, unsigned bkh,
				       unsigned char* &outptr );


#endif /// of __LIBBOKEH_H__
