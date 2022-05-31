#include "RG.h"
#include "decode_io.h"
#include "../../util/rmalloc.h"

//------------------------------------------------------------------------------
// RDB decoder
//------------------------------------------------------------------------------

#define _IO_RDB_LOAD(name, type)                            \
type static _IO_RDB_Load##name                              \
(                                                           \
	const IODecoder *io                                     \
) {                                                         \
	return RedisModule_Load##name((RedisModuleIO*)io->io);  \
}

_IO_RDB_LOAD(Float     , float      );
_IO_RDB_LOAD(Double    , double     );
_IO_RDB_LOAD(Signed    , int64_t    );
_IO_RDB_LOAD(Unsigned  , uint64_t   );
_IO_RDB_LOAD(LongDouble, long double);

static char* _IO_RDB_LoadStringBuffer
(
	const IODecoder *io,
	size_t *len
) {
	return RedisModule_LoadStringBuffer((RedisModuleIO*)io->io, len);
}

//------------------------------------------------------------------------------
// Pipe decoder
//------------------------------------------------------------------------------

#define _IO_PIPE_LOAD(name, type)                           \
type static _IO_PIPE_Load##name                             \
(                                                           \
	const IODecoder *io                                     \
) {                                                         \
	return Pipe_Read##name((Pipe*)io->io);                  \
}

_IO_PIPE_LOAD(Float     , float      );
_IO_PIPE_LOAD(Double    , double     );
_IO_PIPE_LOAD(Signed    , int64_t    );
_IO_PIPE_LOAD(Unsigned  , uint64_t   );
_IO_PIPE_LOAD(LongDouble, long double);

static char* _IO_PIPE_LoadStringBuffer
(
	const IODecoder *io,
	size_t *len
) {
	return Pipe_ReadStringBuffer((Pipe*)io->io, len);
}

//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------

#define _IO_LOAD(name, type)                                \
type IODecoder_Load##name                                   \
(                                                           \
	const IODecoder *io                                     \
) {                                                         \
	return io->Load##name(io);                              \
}

_IO_LOAD(Float     , float      );
_IO_LOAD(Double    , double     );
_IO_LOAD(Signed    , int64_t    );
_IO_LOAD(Unsigned  , uint64_t   );
_IO_LOAD(LongDouble, long double);

char* IODecoder_LoadStringBuffer
(
	const IODecoder *io,
	size_t *len
) {
	return io->LoadStringBuffer(io, len);
}

IODecoder *IODecoder_New
(
	IODecoderType t,
	void *io
) {
	ASSERT(io != NULL);

	IODecoder *decoder = rm_malloc(sizeof(IODecoder));

	decoder->t  = t;
	decoder->io = io;

	// set function pointers
	if(t == IODecoderType_RDB) {
		decoder->LoadFloat        =  _IO_RDB_LoadFloat;
		decoder->LoadDouble       =  _IO_RDB_LoadDouble;
		decoder->LoadSigned       =  _IO_RDB_LoadSigned;
		decoder->LoadUnsigned     =  _IO_RDB_LoadUnsigned;
		decoder->LoadLongDouble   =  _IO_RDB_LoadLongDouble;
		decoder->LoadStringBuffer =  _IO_RDB_LoadStringBuffer;
	} else {
		decoder->LoadFloat        =  _IO_PIPE_LoadFloat;
		decoder->LoadDouble       =  _IO_PIPE_LoadDouble;
		decoder->LoadSigned       =  _IO_PIPE_LoadSigned;
		decoder->LoadUnsigned     =  _IO_PIPE_LoadUnsigned;
		decoder->LoadLongDouble   =  _IO_PIPE_LoadLongDouble;
		decoder->LoadStringBuffer =  _IO_PIPE_LoadStringBuffer;
	}

	return decoder;
}

void IODecoder_Free
(
	IODecoder *decoder
) {
	ASSERT(decoder != NULL);
	rm_free(decoder);
}

