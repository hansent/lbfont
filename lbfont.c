#include <stdio.h>


#include <OpenGL/glu.h>
#include "lbfont.h"
#include "lbshaders.h"


//nothing to do really...always going to get only GL_TRIANGLES becasue  edgeFlag callback is set
void tessBeginCB(GLenum which, LBGlyph* g) { }
void tessEndCB(LBGlyph* g){ }
void tessEdgeFlagCB(GLboolean flag, LBGlyph* g){ }

//hopefully will never get called, only error that is given by GLU
//could be that GLU_TESS_COMBINE callback is needed, i dont think that
//should actually be the case with the font outlines though
void tessErrorCB(GLenum errorCode){
    const GLubyte *errorStr;
    errorStr = gluErrorString(errorCode);
	printf("Tesselator Error!!: %s\n", errorStr);
}

//add the index of teh vertex to the indices array for drawing the trinagles later
void tessVertexCB(Vertex* vertex, LBGlyph* g) {
	GLubyte index = vertex -  g->vertices;
	g->triangle_indices[g->num_triangle_indices++] = index;
}

//tesselate a single glyph from the font (whichever one is loaded into teh glyph slot)
void tesselateFreeTypeOutline(FT_Outline* outline, LBFont* font, LBGlyph* g){
	

	
	// Create and configure a new tessellation object 
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_ERROR_DATA,     tessErrorCB);
    gluTessCallback(tess, GLU_TESS_BEGIN_DATA,     tessBeginCB);
    gluTessCallback(tess, GLU_TESS_END_DATA,       tessEndCB);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, tessEdgeFlagCB);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA,    tessVertexCB);
    

	
	//the cbox is a bounding box that includes the cubic controll points, 
	//used to normalize outline ccordinates
	//FT_BBox     cbox;
	//FT_Outline_Get_CBox(outline, &cbox);
	float w = font->ft_fontface->units_per_EM;//cbox.xMax - cbox.xMin;
	float h = font->ft_fontface->units_per_EM;//cbox.yMax - cbox.yMin;

	//OK, need to tesselate and interpret th eoutline data see:
	//http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
	int i=0, vi=0, contour_index=0, in_curve_index=0, out_curve_index = 0;
    gluTessBeginPolygon(tess, g);
	gluTessBeginContour(tess);
	for (i=0; i<outline->n_points; i++){
		//special case: two B-Spline contorllpoint in a row. Both last one and this one are b-spline,
		//this means we have to insert a "fake" on-edge vertex in right middle, so that the controll
		//points are effectivly all just simple quadratic bezier controll points
		if( ((outline->tags[i]&1) == 0) && ((outline->tags[i-1]&1) == 0)) { 
			GLdouble xi, xj, yi, yj;
			xi = outline->points[i].x ; xj = outline->points[i-1].x;
			yi = outline->points[i].y ; yj = outline->points[i-1].y;
			g->vertices[vi].x = ((xi + xj)/2.0) /w;
			g->vertices[vi].y = ((yi + yj)/2.0) /h;
			g->vertices[vi].z = 1.0 ;
			gluTessVertex(tess, &g->vertices[vi].x, &g->vertices[vi].y);
			vi++; g->num_verts++; //increment vertex pointer, since we added an extra one
		}
		
		//normaly, we just create a vertex from the freetype data
		g->vertices[vi].x = outline->points[i].x /w ;
		g->vertices[vi].y = outline->points[i].y /h ;
		g->vertices[vi].z = 1.0 ;

		//regular 'on-edge' points(not b-spline controll point) need to be tesselated
		//freetype sets the least significant bit to signify these on-edge points
		if (outline->tags[i]&1 ){ 
			gluTessVertex(tess, &g->vertices[vi].x, &g->vertices[vi].y);
		}
		//b-spline controll point
		else { 
			//b-spline controll points are only part of the tesselation if on the inside of teh shape
			//check if its left or right (in/out) between last and next point
			int ax,ay,bx,by, x,y; 
			x = outline->points[i].x; y = outline->points[i].y;
			ax = outline->points[i-1].x; ay = outline->points[i-1].y;
			bx = outline->points[i+1].x; by = outline->points[i+1].y;
			if((bx-ax)*(y-ay) - (by-ay)*(x-ax) < 0){
				gluTessVertex(tess, &g->vertices[vi].x, &g->vertices[vi].y);
				g->inside_ctrl_indices[g->num_inside_ctrl_points++] = vi;
			}else{
				g->outside_ctrl_indices[g->num_outside_ctrl_points++] = vi;
			}
			
		}
		vi++; //done with 1 vertex, so increment vertex index
	
		//done with this contour also?, if so start next one...
		if (i == outline->contours[contour_index]){
			gluTessEndContour(tess);
			gluTessBeginContour(tess);
			g->contours[contour_index] = vi;
			contour_index++;
		}
	}
	gluTessEndContour(tess);
    gluTessEndPolygon(tess);
	
	
		
	
	

}
	

void buildCurveData(LBGlyph* g){
	
	int i=0 , contour_index=0 , inside_index=0 , outside_index=0;
	for (i=0; i<g->num_verts; i++){

		GLubyte before = (i-1);
		GLubyte after = (i+1);
		if (after == g->contours[contour_index]){
			after = g->contours[contour_index-1];
		}
		
		
		if (i == g->inside_ctrl_indices[inside_index] &&  g->num_inside_ctrl_points > 0 ){
			
			Vertex a,b,c;
			a = g->vertices[after];  //point after
			b = g->vertices[before]; //point before
			c = g->vertices[i];      //controll point
			
			//compute texture coords as values to quadratic equation for solving bezier	
			//missusing red channel to mark inside vs outside for now
			a.u = 0.0;  a.v = 0.0;  a.w = -1.0; 
			b.u = 1.0;  b.v = 1.0;  b.w = -1.0; 
			c.u = 0.5;  c.v = 0.0;  c.w = -1.0; 
			
			g->inside_curve_triangles[3*inside_index+0] = a;
			g->inside_curve_triangles[3*inside_index+1] = b;
			g->inside_curve_triangles[3*inside_index+2] = c;

			inside_index++;
		}	
		
		if (i == g->outside_ctrl_indices[outside_index] &&  g->num_outside_ctrl_points > 0){
			
			Vertex a,b,c;
			a = g->vertices[after];  //point after
			b = g->vertices[before]; //point before
			c = g->vertices[i];      //controll point
			
			//compute texture coords as values to quadratic equation for solving bezier			
			a.u = 0.0;  a.v = 0.0;  a.w = 1.0; 
			b.u = 1.0;  b.v = 1.0;  b.w = 1.0; 
			c.u = 0.5;  c.v = 0.0;  c.w = 1.0; 
			
			g->outside_curve_triangles[3*outside_index+0] = a;
			g->outside_curve_triangles[3*outside_index+1] = b;
			g->outside_curve_triangles[3*outside_index+2] = c;		
			outside_index++;
		}
		 //*/
		if (i == g->contours[contour_index])
			contour_index++;
	}
}


void freeLBGlyph(LBGlyph* g){
	free(g->contours); g->contours = NULL;
	free(g->vertices); g->vertices = NULL;
	free(g->triangle_indices); g->triangle_indices = NULL;
	free(g->inside_ctrl_indices); g->inside_ctrl_indices = NULL;
	free(g->outside_ctrl_indices); g->outside_ctrl_indices = NULL;
	free(g->inside_curve_triangles); g->inside_curve_triangles = NULL;
	free(g->outside_curve_triangles); g->outside_curve_triangles = NULL;
	free(g); g = NULL;
}


LBGlyph* initLBGlyph(char c, LBFont* font){
	
	//load char into freetype glyph_slot
	FT_Load_Char( font->ft_fontface, c, FT_LOAD_NO_SCALE );
	float advance = (float)(font->ft_fontface->glyph->advance.x)/(float)font->ft_fontface->units_per_EM;
	
	LBGlyph* g = NULL;
	g = malloc(sizeof(LBGlyph));
	g->advance = advance;
	g->num_verts = font->ft_fontface->glyph->outline.n_points;
	g->num_contours = font->ft_fontface->glyph->outline.n_contours;
	g->num_triangle_indices = 0;
	g->num_inside_ctrl_points = 0;
	g->num_outside_ctrl_points = 0;
	g->contours = calloc(g->num_contours, sizeof(int));
	g->vertices = calloc(2*g->num_verts, sizeof(Vertex)); //new ones are inserted between consecutive contorll points
	g->triangle_indices = calloc(6*g->num_verts, sizeof(GLubyte)); //can be more, up to 3*#verts
	g->inside_ctrl_indices = calloc(g->num_verts, sizeof(GLubyte)); //worst case...all of tehm are controll points
	g->outside_ctrl_indices = calloc(g->num_verts, sizeof(GLubyte)); //worst case...all of tehm are controll points
	
	//parse outline/tesselate the glyph 
	tesselateFreeTypeOutline(&font->ft_fontface->glyph->outline, font, g);
	
	//allocate memory for curve vertices and compute border triangles/texture coords
	g->inside_curve_triangles  = calloc(g->num_inside_ctrl_points * 3, sizeof(Vertex));
	g->outside_curve_triangles = calloc(100, sizeof(Vertex));
	buildCurveData(g);
	
	return g;
}


void drawGlyphData(LBGlyph* glyph){
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glPointSize(5.0);
	
	//draw a white dot for all vertices in the glyph
	glColor3f(.5,.5,.5);
	glDrawArrays(GL_POINTS, 0, glyph->num_verts);
	
	//draw a smaller red dot for all inside bezier control points
	glColor3f(1,0,0);
	glDrawElements(GL_POINTS, glyph->num_inside_ctrl_points, GL_UNSIGNED_BYTE, glyph->inside_ctrl_indices);
	
	//draw a smaller green dot for all outside bezier control points
	glColor3f(0,1,0);
	glDrawElements(GL_POINTS, glyph->num_outside_ctrl_points, GL_UNSIGNED_BYTE, glyph->outside_ctrl_indices);
	
	glDisableClientState(GL_VERTEX_ARRAY);
}


float drawLBGlyph(char c, LBFont* this){
	
	this->glyph = this->glyphs[c];
	
	//draw the inside of the glyph (the part that was tesselated for regular drawing)
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(Vertex), &this->glyph->vertices[0].x);
	glDrawElements(GL_TRIANGLES, this->glyph->num_triangle_indices , GL_UNSIGNED_BYTE, this->glyph->triangle_indices);

	glColor3f(0,0,0);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glUseProgram(this->shader);

	//draw outside curves
	glVertexPointer(3, GL_DOUBLE, sizeof(Vertex), &this->glyph->inside_curve_triangles[0].x);
	glTexCoordPointer(3, GL_DOUBLE, sizeof(Vertex), &this->glyph->inside_curve_triangles[0].u);
	glDrawArrays(GL_TRIANGLES, 0, this->glyph->num_inside_ctrl_points*3);
	
	//draw inside curves (TODO: combine these)
	glVertexPointer(3, GL_DOUBLE, sizeof(Vertex), &this->glyph->outside_curve_triangles[0].x);
	glTexCoordPointer(3, GL_DOUBLE, sizeof(Vertex), &this->glyph->outside_curve_triangles[0].u);
	glDrawArrays(GL_TRIANGLES, 0, this->glyph->num_outside_ctrl_points*3);
	
	glUseProgram(0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	return this->glyph->advance;
	/*
	//draw triangle outlines
	glColor3f(1,1,1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, this->glyph.num_triangle_indices , GL_UNSIGNED_BYTE, this->glyph.triangle_indices);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	 
	//draw teh controll points
	drawGlyphData(&this->glyph);
	*/
	
}


renderLBFontString(char* s, LBFont* this){
	glPushMatrix();
	int i = 0;
	while (s[i] !=  NULL) {
		glTranslated(drawLBGlyph(s[i], this), 0, 0);
		i++;
	}
	glPopMatrix();
}


void initLBFont(char* filename, LBFont* this){
	
	FT_Outline  outline;
	
	//setlocale(LC_ALL, "");

	//load freetype library
    if ( FT_Init_FreeType( &this->ft_library ) )
		printf("error loading font\n");
    //load font face
    if( FT_New_Face(this->ft_library, filename, 0, &this->ft_fontface) )
        printf("Error: loading Font Face from %s\n", filename);
	
	printf("[Face Details]:\n");
	printf("%d char maps available\n", this->ft_fontface->num_charmaps);
	printf("Units per Em: %d\n", this->ft_fontface->units_per_EM);

	
	int i;
	for(i=20;i<128;i++){
		printf("loading glyph: %c (%d)\n", i, i);
		this->glyphs[i] = initLBGlyph(i, this);
	}
	 

	
	//this->glyphs[0] = initLBGlyph(4, this);
	//this->glyphs[0] = initLBGlyph(5, this);
	
	this->glyph = this->glyphs['e'];
	

	this->shader = createShaderProgram("glsl/simple.vs", "glsl/simple.fs");
	
	
	
}


