/*
 *  lbshaders.h
 *  LBFont
 *
 *  Created by Thomas Hansen on 9/22/10.
 *  Copyright 2010 University of iowa. All rights reserved.
 *
 */




char *textFileRead(char *fn) 
{
	FILE *fp;
	char *content = NULL;
	
	int count=0;
	
	if (fn != NULL) {
		
		fp = fopen(fn,"rt");
		
		if (fp != NULL) {
			
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);
			
			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
			
		}
	}
	
	return content;
}



void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}



GLuint createShaderProgram(char* vs_filename, char* fs_filename){
	
	GLuint vs,fs,p;
	
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);	
	
	printf("[Loading Shader] vertex:%s  fragment:%s\n", vs_filename, fs_filename);
	
	const char* vs_src = textFileRead(vs_filename);
	const char* fs_src = textFileRead(fs_filename);
	glShaderSource(vs, 1, &vs_src,NULL);
	glShaderSource(fs, 1, &fs_src,NULL);
	
	glCompileShader(vs);
	glCompileShader(fs);
	
	printShaderInfoLog(vs);
	printShaderInfoLog(fs);
	
	p = glCreateProgram();
	glAttachShader(p,vs);
	glAttachShader(p,fs);
	glLinkProgram(p);
	
	printProgramInfoLog(p);
		free(vs_src);free(fs_src);
	return p;
}