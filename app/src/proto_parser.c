/*******************************************************************************
*  Protobuf SignDoc parser for Ledger
*  Handles SIGN_MODE_DIRECT transactions from firma-js
*******************************************************************************/

#include "proto_parser.h"
#include <string.h>
#include "zxmacros.h"

// On-device builds have cx.h for SHA256; test builds need a stub
#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX) || defined(TARGET_FLEX)
#include "cx.h"
#define PROTO_SHA256(input, inputLen, output, outputLen) cx_hash_sha256(input, inputLen, output, outputLen)
#else
// For test/fuzzing builds, provide a no-op or link against a SHA256 lib
#include <string.h>
static void sha256_stub(const uint8_t *in, uint16_t inLen, uint8_t *out, uint16_t outLen) {
    (void)inLen; (void)outLen;
    MEMZERO(out, 32);
    // Simple non-crypto hash for testing only
    for (uint16_t i = 0; i < inLen && i < 32; i++) {
        out[i] = in[i];
    }
}
#define PROTO_SHA256(input, inputLen, output, outputLen) sha256_stub(input, inputLen, output, outputLen)
#endif

// ---------------------------------------------------------------------------
// Base64 decoder
// ---------------------------------------------------------------------------

static int8_t base64_char_value(uint8_t c) {
    if (c >= 'A' && c <= 'Z') return (int8_t)(c - 'A');
    if (c >= 'a' && c <= 'z') return (int8_t)(c - 'a' + 26);
    if (c >= '0' && c <= '9') return (int8_t)(c - '0' + 52);
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -1;  // padding
    return -2;  // invalid
}

parser_error_t base64_decode_inplace(uint8_t *buf, uint16_t inLen, uint16_t *outLen) {
    *outLen = 0;

    if (inLen == 0) {
        return parser_ok;
    }

    // Base64 length must be multiple of 4
    if (inLen % 4 != 0) {
        return parser_protobuf_invalid_base64;
    }

    uint16_t outPos = 0;

    for (uint16_t i = 0; i < inLen; i += 4) {
        int8_t a = base64_char_value(buf[i]);
        int8_t b = base64_char_value(buf[i + 1]);
        int8_t c = base64_char_value(buf[i + 2]);
        int8_t d = base64_char_value(buf[i + 3]);

        // a and b must be valid (not padding, not invalid)
        if (a < 0 || b < 0) {
            return parser_protobuf_invalid_base64;
        }
        // c can be padding (-1) or valid (>=0), but not invalid (-2)
        if (c == -2 || d == -2) {
            return parser_protobuf_invalid_base64;
        }

        buf[outPos++] = (uint8_t)((a << 2) | (b >> 4));

        if (c >= 0) {
            buf[outPos++] = (uint8_t)(((b & 0x0F) << 4) | (c >> 2));
            if (d >= 0) {
                buf[outPos++] = (uint8_t)(((c & 0x03) << 6) | d);
            }
        }
    }

    *outLen = outPos;
    return parser_ok;
}

// ---------------------------------------------------------------------------
// Protobuf varint decoder (LEB128)
// ---------------------------------------------------------------------------

static parser_error_t read_varint(const uint8_t *buf, uint16_t bufLen,
                                  uint16_t offset, uint64_t *value,
                                  uint8_t *bytesRead) {
    *value = 0;
    *bytesRead = 0;

    for (uint8_t i = 0; i < 10; i++) {  // max 10 bytes for uint64
        if (offset + i >= bufLen) {
            return parser_unexpected_buffer_end;
        }

        uint8_t byte = buf[offset + i];
        *value |= ((uint64_t)(byte & 0x7F)) << (7 * i);
        (*bytesRead)++;

        if ((byte & 0x80) == 0) {
            return parser_ok;
        }
    }

    return parser_protobuf_varint_overflow;
}

// ---------------------------------------------------------------------------
// SignDoc protobuf parser
//
// SignDoc fields (cosmos.tx.v1beta1.SignDoc):
//   field 1 (bytes):  bodyBytes      - wire type 2
//   field 2 (bytes):  authInfoBytes  - wire type 2
//   field 3 (string): chainId        - wire type 2
//   field 4 (uint64): accountNumber  - wire type 0
// ---------------------------------------------------------------------------

parser_error_t proto_parse_signdoc(const uint8_t *buf, uint16_t bufLen,
                                   tx_protobuf_t *out) {
    MEMZERO(out, sizeof(tx_protobuf_t));
    out->decodedLen = bufLen;

    uint16_t offset = 0;

    while (offset < bufLen) {
        // Read field tag (varint)
        uint64_t tag = 0;
        uint8_t tagBytes = 0;
        CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &tag, &tagBytes))
        offset += tagBytes;

        uint8_t fieldNumber = (uint8_t)(tag >> 3);
        uint8_t wireType = (uint8_t)(tag & 0x07);

        switch (fieldNumber) {
            case 1: {
                // bodyBytes (wire type 2 = length-delimited)
                if (wireType != 2) return parser_protobuf_invalid_wire_type;
                uint64_t len = 0;
                uint8_t lenBytes = 0;
                CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &len, &lenBytes))
                offset += lenBytes;

                if (offset + len > bufLen) return parser_unexpected_buffer_end;

                // Compute SHA256 of bodyBytes for blind sign display
                PROTO_SHA256(buf + offset, (uint16_t)len, out->bodyHash, 32);

                offset += (uint16_t)len;
                break;
            }

            case 2: {
                // authInfoBytes (wire type 2 = length-delimited) - skip
                if (wireType != 2) return parser_protobuf_invalid_wire_type;
                uint64_t len = 0;
                uint8_t lenBytes = 0;
                CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &len, &lenBytes))
                offset += lenBytes;

                if (offset + len > bufLen) return parser_unexpected_buffer_end;
                offset += (uint16_t)len;
                break;
            }

            case 3: {
                // chainId (wire type 2 = length-delimited string)
                if (wireType != 2) return parser_protobuf_invalid_wire_type;
                uint64_t len = 0;
                uint8_t lenBytes = 0;
                CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &len, &lenBytes))
                offset += lenBytes;

                if (offset + len > bufLen) return parser_unexpected_buffer_end;
                if (len > 255) return parser_value_out_of_range;

                out->chainId_offset = offset;
                out->chainId_len = (uint8_t)len;

                offset += (uint16_t)len;
                break;
            }

            case 4: {
                // accountNumber (wire type 0 = varint)
                if (wireType != 0) return parser_protobuf_invalid_wire_type;
                uint8_t valBytes = 0;
                CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &out->accountNumber, &valBytes))
                offset += valBytes;
                break;
            }

            default: {
                // Unknown field - skip based on wire type
                switch (wireType) {
                    case 0: {
                        // Varint - read and discard
                        uint64_t dummy = 0;
                        uint8_t dummyBytes = 0;
                        CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &dummy, &dummyBytes))
                        offset += dummyBytes;
                        break;
                    }
                    case 2: {
                        // Length-delimited - skip
                        uint64_t len = 0;
                        uint8_t lenBytes = 0;
                        CHECK_PARSER_ERR(read_varint(buf, bufLen, offset, &len, &lenBytes))
                        offset += lenBytes;
                        if (offset + len > bufLen) return parser_unexpected_buffer_end;
                        offset += (uint16_t)len;
                        break;
                    }
                    case 1: {
                        // Fixed 64-bit
                        if (offset + 8 > bufLen) return parser_unexpected_buffer_end;
                        offset += 8;
                        break;
                    }
                    case 5: {
                        // Fixed 32-bit
                        if (offset + 4 > bufLen) return parser_unexpected_buffer_end;
                        offset += 4;
                        break;
                    }
                    default:
                        return parser_protobuf_invalid_wire_type;
                }
                break;
            }
        }
    }

    // chainId must be present
    if (out->chainId_len == 0) {
        return parser_protobuf_missing_chain_id;
    }

    return parser_ok;
}
