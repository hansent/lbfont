#ifndef LBFONT_H
#define LBFONT_H

#include <OpenGL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H




//dot really need r,g,b right now..but this way its byte aligned to 64
typedef struct {
	GLdouble x, y, z;
	GLdouble u, v, w;
	GLdouble s, t;
} Vertex;


typedef struct {
	float advance;
	int num_verts;
	int num_contours;
	int num_triangle_indices;
	int num_inside_ctrl_points;
	int num_outside_ctrl_points;
	int*      contours;
	Vertex*   vertices;
	Vertex*    inside_curve_triangles;
	Vertex*   outside_curve_triangles;
	GLubyte*  triangle_indices;
	GLubyte*  inside_ctrl_indices;
	GLubyte*  outside_ctrl_indices;
} LBGlyph;


typedef struct {
	FT_Library  ft_library;
    FT_Face     ft_fontface;
	int         num_glyphs;
	LBGlyph*    glyphs[128];
	LBGlyph*    glyph;
	GLuint	    shader;
} LBFont;


void  initLBFont(char*, LBFont*);
void  renderLBFontString(char* s, LBFont*);

#endif //LBFONT_H
