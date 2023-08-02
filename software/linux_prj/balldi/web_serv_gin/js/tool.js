function int16x10_to_float(num)
{
	if(num >= 65536)
		return 0.0;

	if(num > 32767 && num < 65536)
		return (num - 65536) / 10.0;
	else if(num >= 0)
		return num / 10.0;
}


export { int16x10_to_float }