/*******************************************************************************
*  Protobuf SignDoc parser for Ledger
*  Handles SIGN_MODE_DIRECT transactions from firma-js
*******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "common/parser_common.h"

// Parsed Protobuf SignDoc fields
typedef struct {
    uint16_t chainId_offset;    // Offset of chainId string in decoded buffer
    uint8_t  chainId_len;       // Length of chainId string
    uint64_t accountNumber;     // Account number
    uint8_t  bodyHash[32];      // SHA256 of bodyBytes (for blind sign display)
    uint16_t decodedLen;        // Total decoded protobuf length
} tx_protobuf_t;

// Decode base64 string in-place. Output overwrites buf[0..].
// Returns parser_ok on success, sets *outLen to decoded byte count.
parser_error_t base64_decode_inplace(uint8_t *buf, uint16_t inLen, uint16_t *outLen);

// Parse protobuf-encoded SignDoc from raw bytes.
// Extracts chainId, accountNumber, and SHA256(bodyBytes).
parser_error_t proto_parse_signdoc(const uint8_t *buf, uint16_t bufLen,
                                   tx_protobuf_t *out);

#ifdef __cplusplus
}
#endif
