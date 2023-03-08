#include "so_stdio.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

//dimensiune maxima buffer
#define BUFF_SIZE 4096

typedef struct _so_file {
	//buffer to read
	unsigned char *readBuffer;
	//buffer to write
	unsigned char *writeBuffer;
	int fd;
	//position in readBuffer
	int posRead;
	//position in writeBuffer
	int posWrite;
	//how many bytes left to read
	int bytesToRead;
	//how many bytes to write
	int bytesToWrite;
	//cursor in file
	long cursor;
	//last operation
	int lastOp;
	//reach end of file
	int eof;
	//error
	int err;
} SO_FILE;

/*
 * Function that initialize a SO_FILE
 * Receives path to file and mode to open
 * Return the structure of NULL if any error appeared
 */
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int fd;

	//initialize file descriptor
	if (!strcmp(mode, "r"))
		fd = open (pathname, O_RDONLY);
	else if (!strcmp(mode, "r+"))
		fd = open(pathname, O_RDWR);
	else if (!strcmp(mode, "w"))
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (!strcmp(mode, "w+"))
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
	else if (!strcmp(mode, "a"))
		fd = open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0644);
	else if (!strcmp(mode, "a+"))
		fd = open(pathname, O_APPEND | O_RDWR | O_CREAT, 0644);
	else
		return NULL;

	if (fd < 0)
		return NULL;

	/* alocate memory for structure and buffers
	 * check the operations ended successfully
	 */
	SO_FILE *structure = (SO_FILE *)calloc(1, sizeof(SO_FILE));

	if (structure == NULL) {
		close(fd);
		return NULL;
	}

	structure->readBuffer = (unsigned char *)calloc(BUFF_SIZE,
													sizeof(unsigned char));

	if (structure->readBuffer == NULL) {
		close(fd);
		free(structure);
		return NULL;
	}

	structure->writeBuffer = (unsigned char *)calloc(BUFF_SIZE,
													sizeof(unsigned char));

	if (structure->writeBuffer == NULL) {
		close(fd);
		free(structure->readBuffer);
		free(structure);
		return NULL;
	}

	structure->fd = fd;

	return structure;
}

/*
 * Function that close a SO_FILE
 * Receives the structure
 * Return 0 if operation ended well or SO_EOF otherwise
 */
int so_fclose(SO_FILE *stream)
{
	int ret;

	if (!stream)
		return SO_EOF;

	//check if there is any data that needs to be written to the file
	if (stream->lastOp == 1) {
		ret = so_fflush(stream);

		if (ret) {
			stream->err = 1;
			close(stream->fd);

			free(stream->readBuffer);
			free(stream->writeBuffer);
			free(stream);

			return ret;
		}
	}

	//close file descriptor
	ret = close(stream->fd);

	if (ret < 0)
		stream->err = 1;

	//free memory
	free(stream->readBuffer);
	free(stream->writeBuffer);
	free(stream);

	if (ret < 0)
		return SO_EOF;

	return 0;
}

/*
 * Function that gives the file descriptor of the opened file
 * Receives the structure
 * Return fd of file or SO_EOF is argument is NULL
 */
int so_fileno(SO_FILE *stream)
{
	if (!stream)
		return SO_EOF;

	return stream->fd;
}

/*
 * Function that writes the data to output
 * Receives the structure
 * Return 0 if operation ended well or SO_EOF if any error appeared
 */
int so_fflush(SO_FILE *stream)
{
	size_t bytes_written = 0;

	if (!stream)
		return SO_EOF;

	//write till there are no more bytes to be written
	while (bytes_written < stream->bytesToWrite) {
		ssize_t bytes_written_now = write(stream->fd, stream->writeBuffer +
					bytes_written, stream->bytesToWrite - bytes_written);

		//if any error appeared, set error flag and exit function with SO_EOF
		if (bytes_written_now <= 0) {
			stream->err = 1;
			return SO_EOF;
		}

		bytes_written += bytes_written_now;
	}

	//reset position in buffer and number of bytes to be written
	stream->posWrite = 0;
	stream->bytesToWrite = 0;

	return 0;
}

/*
 * Function that modify the cursor in the file
 * Receives the structure of the file, offset to where to move cursor
 * and position from where to set cursor
 * Return 0 if operation ended well or SO_EOF if any error appeared
 */
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int ret;

	//if last operation was read, we need to invalidate buffer
	if (stream->lastOp == 2) {
		memset(stream->readBuffer, 0, BUFF_SIZE);
		stream->posRead = 0;
		stream->bytesToRead = 0;
	}

	//if last operation was write, we need to write the remaining bytes
	if (stream->lastOp == 1) {
		ret = so_fflush(stream);

		if (ret) {
			stream->err = 1;
			return -1;
		}
	}

	//call lseek to modify cursor in file
	ret = lseek(stream->fd, offset, whence);

	if (ret < 0) {
		stream->err = 1;
		return -1;
	}

	//set new cursor stored in file
	stream->cursor = ret;

	return 0;
}

/*
 * Function that gives the position in the file
 * Receives the structure of the file
 * Return the position in file or -1 if stream if NULL
 */
long so_ftell(SO_FILE *stream)
{
	if (!stream)
		return -1;

	return stream->cursor;
}

/*
 * Function that reads to an address nmenb elements of size bytes each
 * Receives the address where to store the bytes, size of an element,
 * number of elements and structure of file
 * Return the number of elements successfully written at ptr
 */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int offset = 0;
	int members = 0;
	unsigned char *buf = (unsigned char *)ptr;
	int c;

	if (!stream)
		return SO_EOF;

	//read bytes while the maximum size hasn't been reached using so_fgetc
	while (members < nmemb) {
		c = so_fgetc(stream);

		//if there was an error, set the error flag
		if (c == SO_EOF) {
			stream->err = 1;

			//if eof was reached, set flag and return number of members read
			if (so_feof(stream))
				return members;

			return 0;
		}

		buf[offset++] = c;
		members = offset / size;
	}

	//return number of members successfully read
	return members;
}

/*
 * Function that writes from an address nmenb elements of size bytes each
 * Receives the address where data is stored, size of an element, number of
 * elements and structure of file
 * Return the number of elements successfully written into file
 */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int offset = 0;
	int members = 0;
	unsigned char *buf = (unsigned char *)ptr;
	int c;

	if (!stream)
		return SO_EOF;

	//write bytes while the maximum size hasn't been reached using so_putc
	while (members < nmemb) {
		c = so_fputc(buf[offset++], stream);

		//if there was an error, set the error flag
		if (c == SO_EOF) {
			stream->err = 1;

			//if eof was reached, set flag and return number of members written
			if (so_feof(stream))
				return members;

			return 0;
		}

		members = offset / size;
	}

	//return number of members successfully written
	return members;
}

/*
 * Function that reads a character from file
 * Receives the structure of the file
 * Return the character read or SO_EOF if there was any error
 */
int so_fgetc(SO_FILE *stream)
{
	int ret;

	if (!stream)
		return SO_EOF;

	//if there is still data not written in file, write it
	if (stream->lastOp == 1) {
		ret = so_fflush(stream);

		if (ret) {
			stream->err = 1;
			return ret;
		}
	}

	//if readBuffer is empty, reset it and read more bytes from file
	if (stream->bytesToRead == 0) {
		memset(stream->readBuffer, 0, BUFF_SIZE);
		stream->posRead = 0;

		//read bytes from file
		ret = read(stream->fd, stream->readBuffer, BUFF_SIZE);

		if (ret < 0) {
			stream->err = 1;
			return SO_EOF;
		}

		if (ret == 0) {
			stream->eof = 1;
			stream->err = 1;
			return SO_EOF;
		}

		//set new lenght of readBuffer
		stream->bytesToRead = ret;
	}

	//decrement the number of bytes available to read from buffer
	stream->bytesToRead--;
	//set last operation as read
	stream->lastOp = 2;
	//increment cursor in file
	stream->cursor++;

	//return character
	return (int)stream->readBuffer[stream->posRead++];
}

/*
 * Function that writes a character to file
 * Receives the character to write and the structure of the file
 * Return the character written or SO_EOF if there was any error
 */
int so_fputc(int c, SO_FILE *stream)
{
	int ret;

	if (!stream)
		return SO_EOF;

	//if writeBuffer is full, clear buffer
	if (stream->bytesToWrite == BUFF_SIZE) {
		ret = so_fflush(stream);

		if (ret) {
			stream->err = 1;
			return ret;
		}
	}

	//store character
	stream->writeBuffer[stream->posWrite] = c;
	//increment number of bytes in buffer
	stream->bytesToWrite++;
	//set last operation as write
	stream->lastOp = 1;
	//increment file cursor
	stream->cursor++;

	//return written character
	return (int)stream->writeBuffer[stream->posWrite++];
}

/*
 * Function that askes if end of file was reached
 * Receives the structure of the file
 * Return 0 if eof was not reached or non-zero value otherwise
 */
int so_feof(SO_FILE *stream)
{
	if (!stream)
		return -1;

	return stream->eof;
}

/*
 * Function that askes if any error appeared
 * Receives the structure of the file
 * Return 0 if there was no error or non-zero value otherwise
 */
int so_ferror(SO_FILE *stream)
{
	if (!stream)
		return -1;

	return stream->err;
}

/*
 * Function not implemented
 */
SO_FILE *so_popen(const char *command, const char *type)
{

	return NULL;
}

/*
 * Function not implemented
 */
int so_pclose(SO_FILE *stream)
{

	return -1;
}
