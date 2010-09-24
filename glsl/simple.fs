
varying vec4 color;
varying vec3 texCoord;

void main()
{
	vec4 frag_color;
	vec2 p, px, py;
   	float fx,fy,sd,alpha;
	
	p = texCoord.xy;
	frag_color = color;

   	px = dFdx( p );
   	py = dFdy( p );
   	fx = 2.0*p.x*px.x - px.y;
   	fy = 2.0*p.x*py.x - py.y;
   	sd = ( p.x*p.x  - p.y ) / sqrt( fx*fx + fy*fy );
   	alpha = 0.5 - sd * texCoord.z;

   	if ( alpha > 1.0 ){
     	frag_color.a  = 1.0;
	}
   	else if ( (alpha < 0.0) ){
		frag_color.a  = 0.0;
	}	
	else{
		frag_color.a  = alpha;
	}

  	//frag_color = vec4(p.x,p.y,0.0,1.0);
	
   
gl_FragColor = frag_color;

}
