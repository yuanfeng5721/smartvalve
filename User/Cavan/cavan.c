#include <cavan.h>

static inline char *cavan_url_encode_append(char *buff, char *buff_end, const char *text)
{
	if (buff + 2 < buff_end) {
		*buff++ = *text++;
		*buff++ = *text++;
		*buff++ = *text++;
	}

	return buff;
}

char *cavan_url_encode(const char *url, char *buff, int size)
{
	char *buff_end = buff + size;

	while (1) {
		char value = *url++;

		switch (value) {
		case 0:
			if (buff < buff_end) {
				*buff = 0;
			}
			return buff;

		case ' ':
			buff = cavan_url_encode_append(buff, buff_end, "%20");
			break;

		case '+':
			buff = cavan_url_encode_append(buff, buff_end, "%2B");
			break;


		case '/':
			buff = cavan_url_encode_append(buff, buff_end, "%2F");
			break;

		case '?':
			buff = cavan_url_encode_append(buff, buff_end, "%3F");
			break;

		case '%':
			buff = cavan_url_encode_append(buff, buff_end, "%25");
			break;

		case '#':
			buff = cavan_url_encode_append(buff, buff_end, "%23");
			break;

		case '&':
			buff = cavan_url_encode_append(buff, buff_end, "%26");
			break;

		case '=':
			buff = cavan_url_encode_append(buff, buff_end, "%3D");
			break;

		default:
			if (buff < buff_end) {
				*buff++ = value;
			}
		}
	}
}

bool cavan_is_space(char value)
{
	switch (value) {
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\f':
	case '\b':
		return true;

	default:
		return false;
	}
}

const char *cavan_filename(const char *pathname)
{
	const char *filename = pathname;

	while (1) {
		switch (*pathname) {
		case 0:
			return filename;

		case '/':
		case '\\':
			filename = pathname + 1;
			break;
		}

		pathname++;
	}
}

char *cavan_text_trim_head(char *text)
{
	while (1) {
		switch (*text) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\f':
		case '\b':
			text++;
			break;

		default:
			return text;
		}
	}
}

char *cavan_text_trim_tail(char *head, char *tail)
{
	while (tail >= head) {
		switch (*tail) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\f':
		case '\b':
			*tail = 0;
			break;

		default:
			return tail;
		}

		tail--;
	}

	return head;
}

int cavan_text_split(char *text, char sep, char *args[], int size)
{
	int index = 0;

	text = cavan_text_trim_head(text);

	if (index < size) {
		args[index++] = text;
	}

	while (1) {
		char value = *text;

		if (value == 0) {
			break;
		}

		if (value == sep && index < size) {
			args[index++] = text + 1;
			*text = 0;
		}

		text++;
	}

	cavan_text_trim_tail(args[index - 1], text - 1);

	return index;
}

bool cavan_text_starts_with(const char *text, const char *prefix)
{
	while (*prefix) {
		if (*text != *prefix) {
			return false;
		}

		prefix++;
		text++;
	}

	return true;
}

bool cavan_text_ends_with(const char *text, u16 len0, const char *suffix, u16 len1)
{
	const char *text_end;

	if (len0 < len1) {
		return false;
	}

	text_end = text + len0;
	text = text_end - len1;

	while (text < text_end) {
		if (*text != *suffix) {
			return false;
		}

		suffix++;
		text++;
	}

	return true;
}

