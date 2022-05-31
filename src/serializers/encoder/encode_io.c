#include "RG.h"
#include "encode_io.h"
#include "../../util/rmalloc.h"

//------------------------------------------------------------------------------
// RDB encoder
//------------------------------------------------------------------------------

#define _IO_RDB_SAVE(name, type)                            \
void static _IO_RDB_Save##name                              \
(                                                           \
	const IOEncoder *io,                                    \
	type value                                              \
) {                                                         \
	RedisModule_Save##name((RedisModuleIO*)io->io, value);  \
}

_IO_RDB_SAVE(Float     , float      );
_IO_RDB_SAVE(Double    , double     );
_IO_RDB_SAVE(Signed    , int64_t    );
_IO_RDB_SAVE(Unsigned  , uint64_t   );
_IO_RDB_SAVE(LongDouble, long double);

void static _IO_RDB_SaveStringBuffer
(
	const IOEncoder *io,
	const char *value,
	size_t len
) {
	RedisModule_SaveStringBuffer((RedisModuleIO*)io->io, value, len);
}

//------------------------------------------------------------------------------
// Pipe encoder
//------------------------------------------------------------------------------

#define _IO_PIPE_SAVE(name, type)                           \
void static _IO_PIPE_Save##name                             \
(                                                           \
	const IOEncoder *io,                                    \
	type value                                              \
) {                                                         \
	Pipe_Write##name((Pipe*)io->io, value);                 \
}

_IO_PIPE_SAVE(Float     , float      );
_IO_PIPE_SAVE(Double    , double     );
_IO_PIPE_SAVE(Signed    , int64_t    );
_IO_PIPE_SAVE(Unsigned  , uint64_t   );
_IO_PIPE_SAVE(LongDouble, long double);

void static _IO_PIPE_SaveStringBuffer
(
	const IOEncoder *io,
	const char *value,
	size_t len
) {
	Pipe_WriteStringBuffer((Pipe*)io->io, value, len);
}

//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------

#define _IO_SAVE(name, type)                                \
void IOEncoder_Save##name                                   \
(                                                           \
	const IOEncoder *io,                                    \
	type value                                              \
) {                                                         \
	io->Save##name(io, value);                              \
}

_IO_SAVE(Float     , float      );
_IO_SAVE(Double    , double     );
_IO_SAVE(Signed    , int64_t    );
_IO_SAVE(Unsigned  , uint64_t   );
_IO_SAVE(LongDouble, long double);

void IOEncoder_SaveStringBuffer
(
	const IOEncoder *io,
	const char *value,
	size_t len
) {
	io->SaveStringBuffer(io, value, len);
}

IOEncoder *IOEncoder_New
(
	IOEncoderType t,
	void *io
) {
	ASSERT(io != NULL);

	IOEncoder *encoder = rm_malloc(sizeof(IOEncoder));

	encoder->t  = t;
	encoder->io = io;

	// set function pointers
	if(t == IOEncoderType_RDB) {
		encoder->SaveFloat        =  _IO_RDB_SaveFloat;
		encoder->SaveDouble       =  _IO_RDB_SaveDouble;
		encoder->SaveSigned       =  _IO_RDB_SaveSigned;
		encoder->SaveUnsigned     =  _IO_RDB_SaveUnsigned;
		encoder->SaveLongDouble   =  _IO_RDB_SaveLongDouble;
		encoder->SaveStringBuffer =  _IO_RDB_SaveStringBuffer;
	} else {
		encoder->SaveFloat        =  _IO_PIPE_SaveFloat;
		encoder->SaveDouble       =  _IO_PIPE_SaveDouble;
		encoder->SaveSigned       =  _IO_PIPE_SaveSigned;
		encoder->SaveUnsigned     =  _IO_PIPE_SaveUnsigned;
		encoder->SaveLongDouble   =  _IO_PIPE_SaveLongDouble;
		encoder->SaveStringBuffer =  _IO_PIPE_SaveStringBuffer;
	}

	return encoder;
}

void IOEncoder_Free
(
	IOEncoder *encoder
) {
	ASSERT(encoder != NULL);
	rm_free(encoder);
}

