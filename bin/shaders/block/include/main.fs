vec2 rotate_uv(vec2 uv)
{
	if(rotation == 1) return vec2(uv.y, 1 - uv.x);
	if(rotation == 2) return 1 - uv;
	if(rotation == 3) return vec2(1 - uv.y, uv.x);
	// assume 0
	return uv;
}

void main()
{
	vec2 coords = get_face_coords(position);
	vec2 uv = fract(coords);
#ifdef USE_COORDS
	coords = floor(coords) + rotate_uv(uv);
	vec4 c = color(coords);
#else
	vec2 uv2 = uv;
	if(face == FACE_RIGHT
	|| face == FACE_TOP
	|| face == FACE_BACK)
	{
		uv2.x = 1 - uv2.x;
	}
	uv2 = rotate_uv(uv2);
	vec4 c = color(uv2);
#endif

	if(min_light > 0.999 && min_light < 1.001)
	{
		FragColor = c;
		return;
	}

	int ix, iy, iz;
	if(face == FACE_RIGHT || face == FACE_LEFT)
	{
		ix = 2; iy = 1; iz = 0;
	}
	else if(face == FACE_TOP || face == FACE_BOTTOM)
	{
		ix = 0; iy = 2; iz = 1;
	}
	else // face == FACE_FRONT || face == FACE_BACK
	{
		ix = 0; iy = 1; iz = 2;
	}

	vec3 l;
	vec3 lpos = relative_position;
	if(face == FACE_LEFT || face == FACE_BOTTOM || face == FACE_BACK)
	{
		lpos[iz] -= 0.5;
	}
	else
	{
		lpos[iz] += 0.5;
	}
	const float div = 32 + 2; // chunk size + 2
#ifndef NO_FANCY_LIGHT_SMOOTHING
	if(light_smoothing == 2)
	{
		vec2 luv = fract(get_face_coords(lpos));
		lpos += 1;
		vec3 pos1, pos2, pos3, pos4;
		pos1[ix] = pos3[ix] = floor(lpos[ix]);
		pos2[ix] = pos4[ix] = ceil(lpos[ix]);
		pos1[iy] = pos2[iy] = floor(lpos[iy]);
		pos3[iy] = pos4[iy] = ceil(lpos[iy]);
		pos1[iz] = pos2[iz] = pos3[iz] = pos4[iz] = lpos[iz];
		vec3 l1 = texture(light, pos1 / div).rgb;
		vec3 l2 = texture(light, pos2 / div).rgb;
		vec3 l3 = texture(light, pos3 / div).rgb;
		vec3 l4 = texture(light, pos4 / div).rgb;
		l = mix(mix(l1, l2, luv.x), mix(l3, l4, luv.x), luv.y);
	}
	else
	{
#endif
		l = texture(light, (lpos + 1) / div).rgb;
#ifndef NO_FANCY_LIGHT_SMOOTHING
	}
#endif

	l *= 255.0 / 16.0;

	float z = -min_light / (min_light - 1);
	c.rgb *= pow((l * l + z) / (1 + z), vec3(1 / 2.2));

	FragColor = clamp(c, 0, 1);
}
