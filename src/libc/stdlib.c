int atoi(const char* str)
{
	int res = 0;

	while (*str >= '0' && *str <= '9') {
		res = res * 10 + (*str - '0');
		str++;
	}
	return res;
};