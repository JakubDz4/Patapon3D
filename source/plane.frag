#version 330 core

out vec3 fragColor;

in vec2 v_texCoord;

uniform float u_barIntensivity;
uniform float u_barStage;
uniform float u_typing;
uniform sampler2D u_Texture;

const float ScreenW=1706.0, ScreenH=1066.0;

const float cirRadius = 0.1 * ScreenH/ScreenW;
const int maxPatapons = 4;
uniform vec2 u_pataponsPos[maxPatapons];
uniform float u_poolHp[maxPatapons];

const float gammaVal = 2.2f;
const float exposure = 1.4f; //ekspozycja, najlepiej duza w nocy, mala w dzien


float sdBox(in vec2 p, in vec2 b)
{
	vec2 d = abs(p) - b;
	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

vec3 borderFunction(vec2 uv)
{
	//vec2 uv = gl_FragCoord.xy / vec2(ScreenW, ScreenH) * 2.0 - 1.0;
	float distance = sdBox(uv, vec2(0.98, 0.98));
	if (distance > 0 && distance < 0.01 * u_barStage)
		return vec3(u_barStage, u_barStage, 0.334) * u_barIntensivity * u_typing;
	return vec3(0.0);
}

float sdCircle(float radius, vec2 circlePos, vec2 uv)
{
	vec2 resRatio = vec2(1.0, ScreenH/ScreenW);

	float distance = length( (uv - circlePos)*resRatio ); 
	//float isInCircle = step(distance, radius);

	float isInCircle = smoothstep(radius, radius*0.8, distance);

	return isInCircle;
}

vec3 PataponColor(vec2 pataponPos, vec2 uv, vec2 circlePos, float scale)
{
	vec2 CoordOffset = ((uv - circlePos)) * scale;

	return texture(u_Texture, (pataponPos + CoordOffset + 1)/2  ).rgb;
}

vec3 setBarColor(float hp, vec2 globalBarPos, vec2 uv)
{
	float hpStartBar = globalBarPos.x-cirRadius + cirRadius*2.0*hp;
	return mix(vec3(0., 1., 0.), vec3(0.0), step(hpStartBar, uv.x));
	//step(hp, barPos.x+cirRadius-uv.x));
}

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(ScreenW, ScreenH) * 2.0 - 1.0;

	vec3 texColor = texture(u_Texture, v_texCoord).rgb;

	vec3 border = borderFunction(uv);

	fragColor = texColor + border;

	int index=-1; 
	for(int i=0; i<maxPatapons; i++)
	{
		//if(u_pataponsPos[i].y < -1. )
		if(u_poolHp[i]<0.0)
		continue;

		index++;
		vec2 cirPos = vec2( -0.97+cirRadius + index*(0.01 + 2*cirRadius), 0.85);

		float isInCircle = sdCircle(cirRadius, cirPos, uv);

		fragColor = mix( fragColor, mix(vec3(-3.0), PataponColor(u_pataponsPos[i], uv, cirPos, .85), isInCircle), isInCircle );

		if(isInCircle>0.)
		continue;

		vec2 barPos = vec2(uv.x - cirPos.x, uv.y - cirPos.y + cirRadius*2);//0.1 to promien circle
		vec3 barColor = setBarColor(u_poolHp[i], cirPos, uv);

		
		//fragColor = mix( vec3(0.0), fragColor, smoothstep(0.0, 0.005, sdBox(barPos, vec2(cirRadius, 0.0125))) );
		//fragColor = mix( barColor, fragColor, smoothstep(0.0, 0.005, sdBox(barPos, vec2(cirRadius, 0.01))) );

		float isBar = sdBox(barPos, vec2(cirRadius, 0.01));

		fragColor = mix(
						mix( barColor, fragColor, smoothstep(0.0, 0.005, isBar) ),
						mix( vec3(0.0), fragColor, smoothstep(0.0, 0.005, sdBox(barPos, vec2(cirRadius, 0.0125))) ),
						step(0.0, isBar)
						);
	}

	vec3 hdrGamma = vec3(1.0) - exp(-fragColor * exposure);
        hdrGamma = pow(hdrGamma, vec3(1.0 / gammaVal));

	fragColor = hdrGamma;
};
