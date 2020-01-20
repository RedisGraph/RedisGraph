#pragma once

#define DELETED_MARKER 0xFF

#define ITEM_HEADER_SIZE 1

#define GET_ITEM_DATA(header) (header? header + ITEM_HEADER_SIZE : header)

#define GET_ITEM_HEADER(item) (item - ITEM_HEADER_SIZE)

#define MARK_HEADER_AS_DELETED(header) (*header = *header | DELETED_MARKER)

#define MARK_HEADER_AS_NOT_DELETED(header) (*header = !DELETED_MARKER)

#define IS_HEADER_DELETED(header) (*header & DELETED_MARKER)
