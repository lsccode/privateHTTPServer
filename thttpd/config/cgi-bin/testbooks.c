#include <stdio.h>
#include <string.h>
#define BOOKSXML \
"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> \
<bookstore> \
<book category=\"children\"> \
<title lang=\"en\">Harry Potter thttpd</title> \
<author>J K. Rowling</author> \
<year>2005</year> \
<price>29.99</price> \
</book> \
<book category=\"cooking\"> \
<title lang=\"en\">Everyday Italian thttpd</title> \
<author>linsicheng thttpd</author> \
<year>2005</year> \
<price>30.00</price> \
</book> \
</bookstore> \
" 
int main(int argc, char *argv[])
{
    printf("Content-Type: text/xml; charset=UTF-8\r\n");
	printf("Content-Length: %u\r\n\r\n",strlen(BOOKSXML));
	printf(BOOKSXML);
    fflush(stdout);
    return 0;
}