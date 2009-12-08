/* $XFree86: xc/programs/Xserver/GL/glx/g_render.c,v 1.12 2004/12/17 16:38:03 tsi Exp $ */
/* DO NOT EDIT - THIS FILE IS AUTOMATICALLY GENERATED */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

#define NEED_REPLIES
#include "glxserver.h"
#include "glxext.h"
#include "g_disptab.h"
#include "g_disptab_EXT.h"
#include "unpack.h"
#include "impsize.h"
#include "singlesize.h"

void __glXDisp_CallList(GLbyte *pc)
{
	glCallList( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_ListBase(GLbyte *pc)
{
	glListBase( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_Begin(GLbyte *pc)
{
	glBegin( 
		*(GLenum   *)(pc + 0)
	);
}

#define __GLX_SWAP_GLbyte(ptr)
#define __GLX_SWAP_GLshort(ptr)  __GLX_SWAP_SHORT(ptr)
#define __GLX_SWAP_GLint(ptr)    __GLX_SWAP_INT(ptr)
#define __GLX_SWAP_GLubyte(ptr)
#define __GLX_SWAP_GLushort(ptr) __GLX_SWAP_SHORT(ptr)
#define __GLX_SWAP_GLuint(ptr)   __GLX_SWAP_INT(ptr)
#define __GLX_SWAP_GLdouble(ptr) __GLX_SWAP_DOUBLE(ptr)
#define __GLX_SWAP_GLfloat(ptr)  __GLX_SWAP_FLOAT(ptr)

#define __GLX_SWAP_GLbyte_ARRAY(ptr,count)   swapEnd = 0;  (void)swapEnd; \
					     swapPC = 0;  (void)swapPC; \
					     sw = 0; (void)sw
#define __GLX_SWAP_GLshort_ARRAY(ptr,count)  __GLX_SWAP_SHORT_ARRAY(ptr,count)
#define __GLX_SWAP_GLint_ARRAY(ptr,count)    __GLX_SWAP_INT_ARRAY(ptr,count)
#define __GLX_SWAP_GLenum_ARRAY(ptr,count)   __GLX_SWAP_INT_ARRAY(ptr,count)
#define __GLX_SWAP_GLubyte_ARRAY(ptr,count)  swapEnd = 0;  (void)swapEnd; \
					     swapPC = 0; (void)swapPC; \
					     sw = 0; (void)sw
#define __GLX_SWAP_GLushort_ARRAY(ptr,count) __GLX_SWAP_SHORT_ARRAY(ptr,count)
#define __GLX_SWAP_GLuint_ARRAY(ptr,count)   __GLX_SWAP_INT_ARRAY(ptr,count)
#define __GLX_SWAP_GLdouble_ARRAY(ptr,count) __GLX_SWAP_DOUBLE_ARRAY(ptr,count)
#define __GLX_SWAP_GLfloat_ARRAY(ptr,count)  __GLX_SWAP_FLOAT_ARRAY(ptr,count)

#ifdef __GLX_ALIGN64
/* If type is not GLdouble, the compiler should optimize this away.
 */
# define GLX_DO_ALIGN_MAGIC(count, type) \
	do { \
	    if ( (sizeof(type) == 8) && ((unsigned long)(pc) & 7)) \
	    { \
	       	__GLX_MEM_COPY(pc-4, pc, (count * sizeof( type ) )); \
	    	pc -= 4; \
	    } \
	} while( 0 )
#else
# define GLX_DO_ALIGN_MAGIC(count, type)
#endif

#define dispatch_template_1( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	GLX_DO_ALIGN_MAGIC( 1, type ); \
	gl ## name ( (type *) pc ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	__GLX_DECLARE_SWAP_VARIABLES; \
	GLX_DO_ALIGN_MAGIC( 1, type ); \
	__GLX_SWAP_ ## type ( pc ); \
	gl ## name ( (type *) pc ); \
    }

#define dispatch_template_3( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	GLX_DO_ALIGN_MAGIC( 3, type ); \
	gl ## name ( (type *) pc ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	__GLX_DECLARE_SWAP_VARIABLES; \
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES; \
	GLX_DO_ALIGN_MAGIC( 3, type ); \
	__GLX_SWAP_ ## type ## _ARRAY(pc, 3); \
	gl ## name ( (type *) pc ); \
    }

#define dispatch_template_4( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	GLX_DO_ALIGN_MAGIC( 4, type ); \
	gl ## name ( (type *) pc ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	__GLX_DECLARE_SWAP_VARIABLES; \
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES; \
	GLX_DO_ALIGN_MAGIC( 4, type ); \
	__GLX_SWAP_ ## type ## _ARRAY(pc, 4); \
	gl ## name ( (type *) pc ); \
    }

#define dispatch_template_4s( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	GLX_DO_ALIGN_MAGIC( 4, type ); \
	gl ## name ( ((type *) pc)[0], ((type *) pc)[1], \
		     ((type *) pc)[2], ((type *) pc)[3] ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	__GLX_DECLARE_SWAP_VARIABLES; \
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES; \
	GLX_DO_ALIGN_MAGIC( 4, type ); \
	__GLX_SWAP_ ## type ## _ARRAY(pc, 4); \
	gl ## name ( ((type *) pc)[0], ((type *) pc)[1], \
		     ((type *) pc)[2], ((type *) pc)[3] ); \
    }

/**
 * \bug All of the enum1 templates need to be updated to handle the case where
 * \c type is \c GLdouble.  When the type is a double, the data comes before
 * the enum.  This is also the reason the invocation of the
 * \c GLX_DO_ALIGN_MAGIC macro was removed.
 */
#define dispatch_template_enum1_1s( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	gl ## name ( *(GLenum *) (pc + 0), \
		     *(type *)   (pc + 4) ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	__GLX_DECLARE_SWAP_VARIABLES; \
	__GLX_SWAP_INT      (pc + 0); \
	__GLX_SWAP_ ## type (pc + 4); \
	gl ## name ( *(GLenum *) (pc + 0), \
		     *(type *)   (pc + 4) ); \
    }

#define dispatch_template_enum1_Vv( name, type ) \
    void __glXDisp_ ## name ( GLbyte * pc ) \
    { \
	gl ## name ( *(GLenum *) (pc + 0), \
		      (type *)   (pc + 4) ); \
    } \
    void __glXDispSwap_ ## name ( GLbyte * pc ) \
    { \
	GLenum pname; GLint compsize; \
	__GLX_DECLARE_SWAP_VARIABLES; \
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES; \
	__GLX_SWAP_INT(pc + 0); \
	pname = *(GLenum *)(pc + 0); \
	compsize = __gl ## name ## _size(pname); \
	if (compsize < 0) compsize = 0; \
	__GLX_SWAP_ ## type ## _ARRAY(pc + 4, compsize); \
	gl ## name ( *(GLenum *) (pc + 0), \
		      (type *)   (pc + 4) ); \
    }

#ifndef MISSING_GL_EXTS
dispatch_template_1( FogCoordfv,         GLfloat )
dispatch_template_1( FogCoorddv,         GLdouble )
dispatch_template_3( SecondaryColor3bv,  GLbyte )
dispatch_template_3( SecondaryColor3sv,  GLshort )
dispatch_template_3( SecondaryColor3iv,  GLint )
dispatch_template_3( SecondaryColor3ubv, GLubyte )
dispatch_template_3( SecondaryColor3usv, GLushort )
dispatch_template_3( SecondaryColor3uiv, GLuint )
dispatch_template_3( SecondaryColor3fv,  GLfloat )
dispatch_template_3( SecondaryColor3dv,  GLdouble )

dispatch_template_4s( BlendFuncSeparate,  GLenum )
#endif /* !MISSING_GL_EXTS */

void __glXDisp_Color3bv(GLbyte *pc)
{
	glColor3bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDisp_Color3dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glColor3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Color3fv(GLbyte *pc)
{
	glColor3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Color3iv(GLbyte *pc)
{
	glColor3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Color3sv(GLbyte *pc)
{
	glColor3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Color3ubv(GLbyte *pc)
{
	glColor3ubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDisp_Color3uiv(GLbyte *pc)
{
	glColor3uiv( 
		(GLuint   *)(pc + 0)
	);
}

void __glXDisp_Color3usv(GLbyte *pc)
{
	glColor3usv( 
		(GLushort *)(pc + 0)
	);
}

void __glXDisp_Color4bv(GLbyte *pc)
{
	glColor4bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDisp_Color4dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glColor4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Color4fv(GLbyte *pc)
{
	glColor4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Color4iv(GLbyte *pc)
{
	glColor4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Color4sv(GLbyte *pc)
{
	glColor4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Color4ubv(GLbyte *pc)
{
	glColor4ubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDisp_Color4uiv(GLbyte *pc)
{
	glColor4uiv( 
		(GLuint   *)(pc + 0)
	);
}

void __glXDisp_Color4usv(GLbyte *pc)
{
	glColor4usv( 
		(GLushort *)(pc + 0)
	);
}

void __glXDisp_EdgeFlagv(GLbyte *pc)
{
	glEdgeFlagv( 
		(GLboolean *)(pc + 0)
	);
}

void __glXDisp_End(GLbyte *pc)
{
	glEnd( 
	);
}

void __glXDisp_Indexdv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	glIndexdv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Indexfv(GLbyte *pc)
{
	glIndexfv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Indexiv(GLbyte *pc)
{
	glIndexiv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Indexsv(GLbyte *pc)
{
	glIndexsv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Normal3bv(GLbyte *pc)
{
	glNormal3bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDisp_Normal3dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glNormal3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Normal3fv(GLbyte *pc)
{
	glNormal3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Normal3iv(GLbyte *pc)
{
	glNormal3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Normal3sv(GLbyte *pc)
{
	glNormal3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_RasterPos2dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glRasterPos2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_RasterPos2fv(GLbyte *pc)
{
	glRasterPos2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_RasterPos2iv(GLbyte *pc)
{
	glRasterPos2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_RasterPos2sv(GLbyte *pc)
{
	glRasterPos2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_RasterPos3dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glRasterPos3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_RasterPos3fv(GLbyte *pc)
{
	glRasterPos3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_RasterPos3iv(GLbyte *pc)
{
	glRasterPos3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_RasterPos3sv(GLbyte *pc)
{
	glRasterPos3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_RasterPos4dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glRasterPos4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_RasterPos4fv(GLbyte *pc)
{
	glRasterPos4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_RasterPos4iv(GLbyte *pc)
{
	glRasterPos4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_RasterPos4sv(GLbyte *pc)
{
	glRasterPos4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Rectdv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glRectdv( 
		(GLdouble *)(pc + 0),
		(GLdouble *)(pc + 16)
	);
}

void __glXDisp_Rectfv(GLbyte *pc)
{
	glRectfv( 
		(GLfloat  *)(pc + 0),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Rectiv(GLbyte *pc)
{
	glRectiv( 
		(GLint    *)(pc + 0),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_Rectsv(GLbyte *pc)
{
	glRectsv( 
		(GLshort  *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDisp_TexCoord1dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	glTexCoord1dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_TexCoord1fv(GLbyte *pc)
{
	glTexCoord1fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_TexCoord1iv(GLbyte *pc)
{
	glTexCoord1iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_TexCoord1sv(GLbyte *pc)
{
	glTexCoord1sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_TexCoord2dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glTexCoord2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_TexCoord2fv(GLbyte *pc)
{
	glTexCoord2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_TexCoord2iv(GLbyte *pc)
{
	glTexCoord2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_TexCoord2sv(GLbyte *pc)
{
	glTexCoord2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_TexCoord3dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glTexCoord3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_TexCoord3fv(GLbyte *pc)
{
	glTexCoord3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_TexCoord3iv(GLbyte *pc)
{
	glTexCoord3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_TexCoord3sv(GLbyte *pc)
{
	glTexCoord3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_TexCoord4dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glTexCoord4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_TexCoord4fv(GLbyte *pc)
{
	glTexCoord4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_TexCoord4iv(GLbyte *pc)
{
	glTexCoord4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_TexCoord4sv(GLbyte *pc)
{
	glTexCoord4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Vertex2dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glVertex2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Vertex2fv(GLbyte *pc)
{
	glVertex2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Vertex2iv(GLbyte *pc)
{
	glVertex2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Vertex2sv(GLbyte *pc)
{
	glVertex2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Vertex3dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glVertex3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Vertex3fv(GLbyte *pc)
{
	glVertex3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Vertex3iv(GLbyte *pc)
{
	glVertex3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Vertex3sv(GLbyte *pc)
{
	glVertex3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_Vertex4dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glVertex4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Vertex4fv(GLbyte *pc)
{
	glVertex4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Vertex4iv(GLbyte *pc)
{
	glVertex4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDisp_Vertex4sv(GLbyte *pc)
{
	glVertex4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDisp_ClipPlane(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 36);
	    pc -= 4;
	}
#endif
	glClipPlane( 
		*(GLenum   *)(pc + 32),
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_ColorMaterial(GLbyte *pc)
{
	glColorMaterial( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDisp_CullFace(GLbyte *pc)
{
	glCullFace( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_Fogf(GLbyte *pc)
{
	glFogf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_Fogfv(GLbyte *pc)
{
	glFogfv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_Fogi(GLbyte *pc)
{
	glFogi( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDisp_Fogiv(GLbyte *pc)
{
	glFogiv( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_FrontFace(GLbyte *pc)
{
	glFrontFace( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_Hint(GLbyte *pc)
{
	glHint( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDisp_Lightf(GLbyte *pc)
{
	glLightf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Lightfv(GLbyte *pc)
{
	glLightfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Lighti(GLbyte *pc)
{
	glLighti( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_Lightiv(GLbyte *pc)
{
	glLightiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_LightModelf(GLbyte *pc)
{
	glLightModelf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_LightModelfv(GLbyte *pc)
{
	glLightModelfv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_LightModeli(GLbyte *pc)
{
	glLightModeli( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDisp_LightModeliv(GLbyte *pc)
{
	glLightModeliv( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_LineStipple(GLbyte *pc)
{
	glLineStipple( 
		*(GLint    *)(pc + 0),
		*(GLushort *)(pc + 4)
	);
}

void __glXDisp_LineWidth(GLbyte *pc)
{
	glLineWidth( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_Materialf(GLbyte *pc)
{
	glMaterialf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Materialfv(GLbyte *pc)
{
	glMaterialfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Materiali(GLbyte *pc)
{
	glMateriali( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_Materialiv(GLbyte *pc)
{
	glMaterialiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_PointSize(GLbyte *pc)
{
	glPointSize( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_PolygonMode(GLbyte *pc)
{
	glPolygonMode( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDisp_Scissor(GLbyte *pc)
{
	glScissor( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12)
	);
}

void __glXDisp_ShadeModel(GLbyte *pc)
{
	glShadeModel( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_TexParameterf(GLbyte *pc)
{
	glTexParameterf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexParameterfv(GLbyte *pc)
{
	glTexParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexParameteri(GLbyte *pc)
{
	glTexParameteri( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_TexParameteriv(GLbyte *pc)
{
	glTexParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_TexEnvf(GLbyte *pc)
{
	glTexEnvf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexEnvfv(GLbyte *pc)
{
	glTexEnvfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexEnvi(GLbyte *pc)
{
	glTexEnvi( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_TexEnviv(GLbyte *pc)
{
	glTexEnviv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_TexGend(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glTexGend( 
		*(GLenum   *)(pc + 8),
		*(GLenum   *)(pc + 12),
		*(GLdouble *)(pc + 0)
	);
}

void __glXDisp_TexGendv(GLbyte *pc)
{
#ifdef __GLX_ALIGN64
	GLenum pname;
	GLint cmdlen;
	GLint compsize;

	pname = *(GLenum *)(pc + 4);
	compsize = __glTexGendv_size(pname);
	if (compsize < 0) compsize = 0;
	cmdlen = __GLX_PAD(8+compsize*8);
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, cmdlen);
	    pc -= 4;
	}
#endif

	glTexGendv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLdouble *)(pc + 8)
	);
}

void __glXDisp_TexGenf(GLbyte *pc)
{
	glTexGenf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexGenfv(GLbyte *pc)
{
	glTexGenfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_TexGeni(GLbyte *pc)
{
	glTexGeni( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_TexGeniv(GLbyte *pc)
{
	glTexGeniv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_InitNames(GLbyte *pc)
{
	glInitNames( 
	);
}

void __glXDisp_LoadName(GLbyte *pc)
{
	glLoadName( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_PassThrough(GLbyte *pc)
{
	glPassThrough( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_PopName(GLbyte *pc)
{
	glPopName( 
	);
}

void __glXDisp_PushName(GLbyte *pc)
{
	glPushName( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_DrawBuffer(GLbyte *pc)
{
	glDrawBuffer( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_Clear(GLbyte *pc)
{
	glClear( 
		*(GLbitfield *)(pc + 0)
	);
}

void __glXDisp_ClearAccum(GLbyte *pc)
{
	glClearAccum( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLfloat  *)(pc + 12)
	);
}

void __glXDisp_ClearIndex(GLbyte *pc)
{
	glClearIndex( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_ClearColor(GLbyte *pc)
{
	glClearColor( 
		*(GLclampf *)(pc + 0),
		*(GLclampf *)(pc + 4),
		*(GLclampf *)(pc + 8),
		*(GLclampf *)(pc + 12)
	);
}

void __glXDisp_ClearStencil(GLbyte *pc)
{
	glClearStencil( 
		*(GLint    *)(pc + 0)
	);
}

void __glXDisp_ClearDepth(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	glClearDepth( 
		*(GLclampd *)(pc + 0)
	);
}

void __glXDisp_StencilMask(GLbyte *pc)
{
	glStencilMask( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_ColorMask(GLbyte *pc)
{
	glColorMask( 
		*(GLboolean *)(pc + 0),
		*(GLboolean *)(pc + 1),
		*(GLboolean *)(pc + 2),
		*(GLboolean *)(pc + 3)
	);
}

void __glXDisp_DepthMask(GLbyte *pc)
{
	glDepthMask( 
		*(GLboolean *)(pc + 0)
	);
}

void __glXDisp_IndexMask(GLbyte *pc)
{
	glIndexMask( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDisp_Accum(GLbyte *pc)
{
	glAccum( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_Disable(GLbyte *pc)
{
	glDisable( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_Enable(GLbyte *pc)
{
	glEnable( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_PopAttrib(GLbyte *pc)
{
	glPopAttrib( 
	);
}

void __glXDisp_PushAttrib(GLbyte *pc)
{
	glPushAttrib( 
		*(GLbitfield *)(pc + 0)
	);
}

void __glXDisp_MapGrid1d(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 20);
	    pc -= 4;
	}
#endif
	glMapGrid1d( 
		*(GLint    *)(pc + 16),
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8)
	);
}

void __glXDisp_MapGrid1f(GLbyte *pc)
{
	glMapGrid1f( 
		*(GLint    *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_MapGrid2d(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 40);
	    pc -= 4;
	}
#endif
	glMapGrid2d( 
		*(GLint    *)(pc + 32),
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLint    *)(pc + 36),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24)
	);
}

void __glXDisp_MapGrid2f(GLbyte *pc)
{
	glMapGrid2f( 
		*(GLint    *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLfloat  *)(pc + 16),
		*(GLfloat  *)(pc + 20)
	);
}

void __glXDisp_EvalCoord1dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	glEvalCoord1dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_EvalCoord1fv(GLbyte *pc)
{
	glEvalCoord1fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_EvalCoord2dv(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glEvalCoord2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_EvalCoord2fv(GLbyte *pc)
{
	glEvalCoord2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_EvalMesh1(GLbyte *pc)
{
	glEvalMesh1( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_EvalPoint1(GLbyte *pc)
{
	glEvalPoint1( 
		*(GLint    *)(pc + 0)
	);
}

void __glXDisp_EvalMesh2(GLbyte *pc)
{
	glEvalMesh2( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16)
	);
}

void __glXDisp_EvalPoint2(GLbyte *pc)
{
	glEvalPoint2( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDisp_AlphaFunc(GLbyte *pc)
{
	glAlphaFunc( 
		*(GLenum   *)(pc + 0),
		*(GLclampf *)(pc + 4)
	);
}

void __glXDisp_BlendFunc(GLbyte *pc)
{
	glBlendFunc( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDisp_LogicOp(GLbyte *pc)
{
	glLogicOp( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_StencilFunc(GLbyte *pc)
{
	glStencilFunc( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLuint   *)(pc + 8)
	);
}

void __glXDisp_StencilOp(GLbyte *pc)
{
	glStencilOp( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLenum   *)(pc + 8)
	);
}

void __glXDisp_DepthFunc(GLbyte *pc)
{
	glDepthFunc( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_PixelZoom(GLbyte *pc)
{
	glPixelZoom( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_PixelTransferf(GLbyte *pc)
{
	glPixelTransferf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_PixelTransferi(GLbyte *pc)
{
	glPixelTransferi( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDisp_PixelMapfv(GLbyte *pc)
{
	glPixelMapfv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_PixelMapuiv(GLbyte *pc)
{
	glPixelMapuiv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLuint   *)(pc + 8)
	);
}

void __glXDisp_PixelMapusv(GLbyte *pc)
{
	glPixelMapusv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLushort *)(pc + 8)
	);
}

void __glXDisp_ReadBuffer(GLbyte *pc)
{
	glReadBuffer( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_CopyPixels(GLbyte *pc)
{
	glCopyPixels( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12),
		*(GLenum   *)(pc + 16)
	);
}

void __glXDisp_DepthRange(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	glDepthRange( 
		*(GLclampd *)(pc + 0),
		*(GLclampd *)(pc + 8)
	);
}

void __glXDisp_Frustum(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 48);
	    pc -= 4;
	}
#endif
	glFrustum( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24),
		*(GLdouble *)(pc + 32),
		*(GLdouble *)(pc + 40)
	);
}

void __glXDisp_LoadIdentity(GLbyte *pc)
{
	glLoadIdentity( 
	);
}

void __glXDisp_LoadMatrixf(GLbyte *pc)
{
	glLoadMatrixf( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_LoadMatrixd(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 128);
	    pc -= 4;
	}
#endif
	glLoadMatrixd( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_MatrixMode(GLbyte *pc)
{
	glMatrixMode( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_MultMatrixf(GLbyte *pc)
{
	glMultMatrixf( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDisp_MultMatrixd(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 128);
	    pc -= 4;
	}
#endif
	glMultMatrixd( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_Ortho(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 48);
	    pc -= 4;
	}
#endif
	glOrtho( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24),
		*(GLdouble *)(pc + 32),
		*(GLdouble *)(pc + 40)
	);
}

void __glXDisp_PopMatrix(GLbyte *pc)
{
	glPopMatrix( 
	);
}

void __glXDisp_PushMatrix(GLbyte *pc)
{
	glPushMatrix( 
	);
}

void __glXDisp_Rotated(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	glRotated( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24)
	);
}

void __glXDisp_Rotatef(GLbyte *pc)
{
	glRotatef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLfloat  *)(pc + 12)
	);
}

void __glXDisp_Scaled(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glScaled( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16)
	);
}

void __glXDisp_Scalef(GLbyte *pc)
{
	glScalef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Translated(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	glTranslated( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16)
	);
}

void __glXDisp_Translatef(GLbyte *pc)
{
	glTranslatef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_Viewport(GLbyte *pc)
{
	glViewport( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12)
	);
}

void __glXDisp_PolygonOffset(GLbyte *pc)
{
	glPolygonOffset( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_CopyTexImage1D(GLbyte *pc)
{
	glCopyTexImage1D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20),
		*(GLint    *)(pc + 24)
	);
}

void __glXDisp_CopyTexImage2D(GLbyte *pc)
{
	glCopyTexImage2D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20),
		*(GLsizei  *)(pc + 24),
		*(GLint    *)(pc + 28)
	);
}

void __glXDisp_CopyTexSubImage1D(GLbyte *pc)
{
	glCopyTexSubImage1D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20)
	);
}

void __glXDisp_CopyTexSubImage2D(GLbyte *pc)
{
	glCopyTexSubImage2D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLint    *)(pc + 20),
		*(GLsizei  *)(pc + 24),
		*(GLsizei  *)(pc + 28)
	);
}

void __glXDisp_BindTexture(GLbyte *pc)
{
	glBindTexture( 
		*(GLenum   *)(pc + 0),
		*(GLuint   *)(pc + 4)
	);
}

void __glXDisp_PrioritizeTextures(GLbyte *pc)
{
	GLsizei n;

	n = *(GLsizei *)(pc + 0);

	glPrioritizeTextures( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4),
		(GLclampf *)(pc + 4+n*4)
	);
}

void __glXDisp_Indexubv(GLbyte *pc)
{
	glIndexubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDisp_BlendColor(GLbyte *pc)
{
	glBlendColor( 
		*(GLclampf *)(pc + 0),
		*(GLclampf *)(pc + 4),
		*(GLclampf *)(pc + 8),
		*(GLclampf *)(pc + 12)
	);
}

void __glXDisp_BlendEquation(GLbyte *pc)
{
	glBlendEquation( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_ColorTableParameterfv(GLbyte *pc)
{
	glColorTableParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_ColorTableParameteriv(GLbyte *pc)
{
	glColorTableParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_CopyColorTable(GLbyte *pc)
{
	glCopyColorTable( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDisp_CopyColorSubTable(GLbyte *pc)
{
	glCopyColorSubTable( 
		*(GLenum   *)(pc + 0),
		*(GLsizei  *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDisp_ConvolutionParameterf(GLbyte *pc)
{
	glConvolutionParameterf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_ConvolutionParameterfv(GLbyte *pc)
{
	glConvolutionParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDisp_ConvolutionParameteri(GLbyte *pc)
{
	glConvolutionParameteri( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDisp_ConvolutionParameteriv(GLbyte *pc)
{
	glConvolutionParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDisp_CopyConvolutionFilter1D(GLbyte *pc)
{
	glCopyConvolutionFilter1D( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDisp_CopyConvolutionFilter2D(GLbyte *pc)
{
	glCopyConvolutionFilter2D( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16),
		*(GLsizei  *)(pc + 20)
	);
}

void __glXDisp_Histogram(GLbyte *pc)
{
	glHistogram( 
		*(GLenum   *)(pc + 0),
		*(GLsizei  *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLboolean *)(pc + 12)
	);
}

void __glXDisp_Minmax(GLbyte *pc)
{
	glMinmax( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLboolean *)(pc + 8)
	);
}

void __glXDisp_ResetHistogram(GLbyte *pc)
{
	glResetHistogram( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_ResetMinmax(GLbyte *pc)
{
	glResetMinmax( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_CopyTexSubImage3D(GLbyte *pc)
{
	glCopyTexSubImage3D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLint    *)(pc + 20),
		*(GLint    *)(pc + 24),
		*(GLsizei  *)(pc + 28),
		*(GLsizei  *)(pc + 32)
	);
}

void __glXDisp_ActiveTextureARB(GLbyte *pc)
{
	glActiveTextureARB( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDisp_MultiTexCoord1dvARB(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 12);
	    pc -= 4;
	}
#endif
	glMultiTexCoord1dvARB( 
		*(GLenum   *)(pc + 8),
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_MultiTexCoord1fvARB(GLbyte *pc)
{
	glMultiTexCoord1fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord1ivARB(GLbyte *pc)
{
	glMultiTexCoord1ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord1svARB(GLbyte *pc)
{
	glMultiTexCoord1svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord2dvARB(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 20);
	    pc -= 4;
	}
#endif
	glMultiTexCoord2dvARB( 
		*(GLenum   *)(pc + 16),
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_MultiTexCoord2fvARB(GLbyte *pc)
{
	glMultiTexCoord2fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord2ivARB(GLbyte *pc)
{
	glMultiTexCoord2ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord2svARB(GLbyte *pc)
{
	glMultiTexCoord2svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord3dvARB(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 28);
	    pc -= 4;
	}
#endif
	glMultiTexCoord3dvARB( 
		*(GLenum   *)(pc + 24),
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_MultiTexCoord3fvARB(GLbyte *pc)
{
	glMultiTexCoord3fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord3ivARB(GLbyte *pc)
{
	glMultiTexCoord3ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord3svARB(GLbyte *pc)
{
	glMultiTexCoord3svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord4dvARB(GLbyte *pc)
{

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 36);
	    pc -= 4;
	}
#endif
	glMultiTexCoord4dvARB( 
		*(GLenum   *)(pc + 32),
		(GLdouble *)(pc + 0)
	);
}

void __glXDisp_MultiTexCoord4fvARB(GLbyte *pc)
{
	glMultiTexCoord4fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord4ivARB(GLbyte *pc)
{
	glMultiTexCoord4ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDisp_MultiTexCoord4svARB(GLbyte *pc)
{
	glMultiTexCoord4svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}


/*
 * Extensions
 */

#ifndef MISSING_GL_EXTS

void __glXDisp_PointParameterfARB(GLbyte *pc)
{
	glPointParameterfARB(
		*(GLenum *)(pc + 0),
		*(GLfloat *)(pc + 4)
	);
}


void __glXDisp_PointParameterfvARB(GLbyte *pc)
{
	glPointParameterfvARB(
		*(GLenum *)(pc + 0),
		(GLfloat *)(pc + 4)
	);
}

#ifdef __DARWIN__
#define __glPointParameterivNV_size __glPointParameteriv_size
dispatch_template_enum1_1s(PointParameteriNV,  GLint)
dispatch_template_enum1_Vv(PointParameterivNV, GLint)
#else
dispatch_template_enum1_1s(PointParameteri,  GLint)
dispatch_template_enum1_Vv(PointParameteriv, GLint)
#endif

void __glXDisp_ActiveStencilFaceEXT(GLbyte *pc)
{
	glActiveStencilFaceEXT(
		*(GLenum *)(pc + 0)
	);
}

void __glXDisp_WindowPos3fARB(GLbyte *pc)
{
	glWindowPos3fARB(
		*(GLfloat *)(pc + 0),
		*(GLfloat *)(pc + 4),
		*(GLfloat *)(pc + 8)
	);
}
#endif /* !MISSING_GL_EXTS */

void __glXDisp_SampleCoverageARB(GLbyte *pc)
{
	glSampleCoverageARB(
		*(GLfloat *)(pc + 0),
		*(GLboolean *)(pc + 4)
	);
}
