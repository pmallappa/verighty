/*
 * ringbuf.c : Implementing a Ringbuffer for Linux application (multithreaded)
 * Author: Prem Mallappa <prem.mallappa@gmail.com>
 *
 * Description: Circular buffer implementation. Sort of generic implementation.
 *               Allows multiple circular buffers to exist, all the needed
 *               memory is allocated at the beginning to avoid any runtime
 *               errors.
 * - Returns an Handle, basically array index to cyclic buffer pointer
 * - All the clients making use of this has to retain the Handle.
 * - All the buffer related calls are made with the Handle.
 * - We dont allow destroying a buffer at this time
 */

#include <stdio.h>
#include <stdlib.h>             /* for memcpy */
#include <string.h>
#include <pthread.h>

typedef enum
{
	RINGBUF_STATE_EMPTY  = 0x1,
	RINGBUF_STATE_FULL   = 0x2,
	RINGBUF_STATE_NORMAL = 0x4,
} ringbuf_state_t;

typedef enum {
        /* Flags to react when buffer is full */
        RBUF_FLAG_OVERWRITE = 1 << 0, /* Overwrite the buffer when full */
        RBUF_FLAG_ALLOCATE  = 1 << 1, /* Allocate more buffer on full */
        RBUF_FLAG_FULL_MASK = (RBUF_FLAG_ALLOCATE | RBUF_FLAG_OVERWRITE),
} ringbuf_flag_t;

typedef struct {
	pthread_semaphore_t  lock;
        uint8_t             *data;     /* Data buffer */
        ringbuf_state_t      bufstate; /*  */
        ringbuf_flag_t       flags;    /*  */
	volatile uint16_t    writepos; /* Max buf size is 64K */
	volatile uint16_t    readpos;
	uint32_t             bufsize;
} ringbuf_info_t;

typedef void * ringbuf_t;

T_VOID ringbuf_setflag(ringbuf_t rb_handle, ringbuf_flag_t flags)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;
        if ((cci->flags | flags) && (RBUF_FLAGS_ALLOCATE | RBUF_FLAG_OVERWRITE) ) {
                printf("OVERWRITE and ALLOCATE cannot be used together");
                return;
        }
	cci->flags |= flags;
}

T_VOID ringbuf_clearflag(ringbuf_t rb_handle, ringbuf_flag_t flags)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;

	cci->flags &= ~flags;
}

/*
 * Function:	cybuf_clear
 * Description: Clear all the read/write heads
 * ringbuf_t rb_handle: the Handle
 * Return: 	None
 */
T_VOID ringbuf_clear(ringbuf_t rb_handle)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;
	cci->bufstate = RINGBUF_EMPTY;
	cci->readpos = 0;
	cci->writepos = 0;
}

/*
 * Function:         ringbuf_create
 * Description:      create ringbuf
 * ringbuf_t rb_handle: the ring buffer id
 * uint32 size: the size you want to create
 * Return:           create succeed or not
 */
ringbuf_t ringbuf_create(uint32 size)
{
	ringbuf_info_t *cci;

	if(size > ONE_BUFFER_SIZE)
	{
		sysDebugOutput("RingBuf: size too large\n");
		goto err_out;
	}

	if (ringbuf_idx ==  MAX_RINGBUF_COUNT)
	{
		sysDebugOutput("Ringlic buffer maxed out\n");
		goto err_out;
	}

	cci = Fwl_Malloc(sizeof(*cci));
	if (cci == AK_NULL)
	{
		sysDebugOutput("Not enough memory for ringlic buffer metadata\n");
		return AK_NULL;
	}

	cci->bufstate = RINGBUF_EMPTY;
	cci->readpos = 0;
	cci->writepos = 0;
	cci->flags = 0;

	cci->lock = AK_Create_Semaphore(1, AK_PRIORITY);

	cci->ptr = Fwl_Malloc(size);
	if (cci->ptr == AK_NULL)
	{
		sysDebugOutput("Not enough memory for ringlic buffer\n");
		goto free_out;
	}

	cci->bufsize = size;
	ringbuf_idx++;

	sysDebugOutput("Created ringlic print buffer of size :%d\n", size);
	return (ringbuf_t)cci;

free_out:
	Fwl_Free(cci);
err_out:
	return (ringbuf_t)AK_NULL;
}

/*
 * Function:         ringbuf_getblanksize
 * Description:      get ringbuf blank size
 * Input:            ringbuf_t rb_handle: the ring buffer id
 * Return:           blank size
 */
uint32 ringbuf_getblanksize(ringbuf_t rb_handle)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;
	if (cci == AK_NULL)
	{
		return 0;
	}

	if (cci->writepos >= cci->readpos)
	{
		return (cci->bufsize + cci->readpos - cci->writepos);
	}

	return (cci->readpos - cci->writepos);
}

/*
 * Function:         ringbuf_getdatasize
 * Description:      get ringbuf data size
 * Input:            ringbuf_t rb_handle: the ring buffer id
 * Return:           data size
*/
uint32 ringbuf_getdatasize(ringbuf_t rb_handle)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;

	if (cci == AK_NULL)
	{
		return 0;
	}

	if (cci->bufstate == RINGBUF_FULL)
		return cci->bufsize;

	return (cci->bufsize - ringbuf_getblanksize(rb_handle));
}


static T_VOID ringbuf_print_stats(ringbuf_info_t *cci)
{
	sysDebugOutput("readpos:%d writepos:%d state:%d bufsize:%d\n",
			cci->readpos, cci->writepos, cci->bufstate, cci->bufsize);
}

/*
 * Function:		ringbuf_updatewritepos
 * Description:		Updates the write pointers and status
 * T_RINGBUFINFO *cci: 	Ringlic buffer handle
 * uint32 size:		Size to be updated with
 * Return:		None
 *
 * This function is called with lock holding
 */
static T_VOID ringbuf_updatewritepos(ringbuf_info_t *cci, uint32 size)
{
	cci->writepos += size;
	if (ringbuf_getdatasize((ringbuf_t)cci) == cci->bufsize)
		cci->bufstate = RINGBUF_FULL;
	else
		cci->bufstate = RINGBUF_NORMAL;

	cci->writepos %= cci->bufsize;

	ringbuf_print_stats(cci);
}

/*
 * Function:         ringbuf_write_data
 * Description:      write buf data into ringbuf
 * ringbuf_t rb_handle: the ring buffer id
 * buf: the data buf you want to write from
 * uint32 size: the size you want to write
 * Return:           write size
 */
uint32 ringbuf_write_data(ringbuf_t rb_handle, const uint8* buf, uint32 size)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;
	uint32 size1 = 0, size2 = 0;

	//sysDebugOutput("%s id:%p, size:%d\n", __func__, cci, size);
	if (RINGBUF_FULL == cci->bufstate)
		goto out;

	if (AK_Obtain_Semaphore(cci->lock, AK_SUSPEND) < 0)
	{
		goto out;
	}

	/* Can't fit in the data */
	if ((ringbuf_getblanksize(rb_handle) < size) && 
			!(cci->flags & CB_FLAG_OVERWRITE))
		goto out;

	size1 = cci->bufsize - cci->writepos;
	if (size1 > size)
		size1 = size;

	if (size1)
	{
		memcpy(&cci->ptr[cci->writepos], buf, size1);
		ringbuf_updatewritepos(cci, size1);
	}

	size2 = size - size1;
	if (size2)
	{
		memcpy(&cci->ptr[cci->writepos], buf + size1, size2);
		ringbuf_updatewritepos(cci, size2);
	}

out:
	AK_Release_Semaphore(cci->lock);
	return (size1 + size2);
}

static T_VOID ringbuf_updatereadpos(ringbuf_info_t *cci, uint32 size)
{
	cci->readpos += size;

	cci->readpos %= cci->bufsize;

	if (ringbuf_getdatasize((ringbuf_t)cci) == 0)
		cci->bufstate = RINGBUF_EMPTY;
	else
		cci->bufstate = RINGBUF_NORMAL;

	//ringbuf_print_stats(cci);
}

/*
 * Function:         ringbuf_read_data
 * Description:      Read ringbuf data into buffer
 * ringbuf_t rb_handle: the ring buffer id
 * buf: the data buf you want to read into
 * uint32 size: the size you want to read
 * Return:           actual read size
 */
uint32 ringbuf_read_data(ringbuf_t rb_handle, uint8* buf, uint32 size)
{
	uint32 datasize = ringbuf_getdatasize(rb_handle);
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;
	uint32 size1, size2;

	sysDebugOutput("%s id:%p, size:%d\n", __func__, cci, size);
	if (cci == AK_NULL)
	{
		return 0;
	}

	AK_Obtain_Semaphore(cci->lock, AK_SUSPEND);
	if (RINGBUF_EMPTY == cci->bufstate)
	{
		sysDebugOutput("%s buffer EMPTY\n", __func__);
		return 0;
	}

	if (size > datasize)
	{
		size = datasize;
	}

	size1 = cci->bufsize - cci->readpos;
	if (size1 > size)
		size1 = size;

	ringbuf_print_stats(cci);
	if (size1)
	{
		memcpy(buf, &cci->ptr[cci->readpos], size1);
		ringbuf_updatereadpos(cci, size1);
	}

	ringbuf_print_stats(cci);
	size2 = size - size1;
	if (size2)
	{
		memcpy(buf + size1, &cci->ptr[cci->readpos], size2);
		ringbuf_updatereadpos(cci, size2);
	}

	ringbuf_print_stats(cci);
	AK_Release_Semaphore(cci->lock);

	return (size1 + size2);
}


/*
 * Function:         ringbuf_destory
 * Description:      destory the ringbuf
 * ringbuf_t rb_handle: the ring buffer id
 * uint32 size: the size you want to destory
 * Return:           SUCCEED:AK_TRUE   FAILED:AK_FALSE
 */
T_BOOL ringbuf_destory(ringbuf_t rb_handle, uint32 size)
{
	ringbuf_info_t *cci = (ringbuf_info_t*)rb_handle;

	if (cci == AK_NULL)
	{
		sysDebugOutput("Invalid ringbuf pointer to destroy\n");
		return AK_FALSE;
	}

	// Not supported right now
	return AK_TRUE;

	Fwl_Free(cci->ptr);
	Fwl_Free(cci);

	return AK_TRUE;
}


