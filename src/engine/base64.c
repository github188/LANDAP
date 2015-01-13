
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "base64.h"


/* b64_table
 *   base64 encode reference table
 */
INT1 b64_table [] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

/* base64_calculate_encrypted_length
 *   fucntion to calculate length of encoded string
 *
 *   strIn : input string to be encoded
 *
 *   return : length of encoded string
 */
INT4
base64_calculate_encrypted_length
(
    INT1* strIn
)
{
    INT4 iEncodeLen = 0;
    INT4 iRemain = 0;

    if (strIn == NULL || strlen(strIn) == 0) {
        return 0;
    }

    iRemain = strlen(strIn) % 3;
    switch (iRemain) {
        case 1:
        case 2:
            iEncodeLen = ((strlen(strIn) / 3) + 1) * 4;
            break;
        case 0:
            iEncodeLen = (strlen(strIn) / 3) * 4;
            break;
    }
    return iEncodeLen;
}

/* base64_calculate_decrypted_length
 *   function to calculate length of decoded string
 *
 *   strIn : string is going to be decoded
 *
 *   return : length of decoded string
 */
INT4
base64_calculate_decrypted_length
(
    INT1* strIn
)
{
    INT4 iDecodeLen = 0;
    INT4 iInLen = 0;

    if (strIn == NULL || strlen(strIn) == 0) {
        return 0;
    }

    iInLen= strlen(strIn);
    iDecodeLen = iInLen / 4 * 3;
    if (*(strIn + iInLen - 1) == '=') {
        iDecodeLen--;
        if (*(strIn + iInLen - 2) == '=') {
            iDecodeLen--;
        }
    }

    return iDecodeLen;
}

/* base64_encode
 *   function to encode a string according to base 64 algorithm
 *
 *   strIn : input string to be encoded
 *   strOut : buffer for output string
 *   iOutBufSize : size of strOut buffer
 *
 *   return : actual length of output string
 */
INT4
base64_encode
(
    INT1* strIn,
    INT1* strOut,
    INT4 iOutBufSize
)
{
    INT4 iInLen = 0;
    INT4 n = 0;
    INT4 in_idx = 0;
    INT4 out_idx = 0;
    INT1 in_word[3];
    INT1 out_word[4];

    if (strIn == NULL || strOut == NULL) {
        return -1;
    }
    if (strlen(strIn) == 0) {
        return -2;
    }
    iInLen = strlen(strIn);
    in_word[0] = in_word[1] = in_word[2] = 0;
    n = 0;
    for (in_idx = 0; in_idx < iInLen; in_idx++) {
        in_word[n] = *(strIn + in_idx);
        n++;

        if (n == 3 || in_idx == iInLen - 1) {
            out_word[0] = b64_table[in_word[0] >> 2];
            out_word[1] = b64_table[((in_word[0] & 3) << 4) | (in_word[1] >> 4)];
            out_word[2] = b64_table[((in_word[1] & 0xF) << 2) | (in_word[2] >> 6)];
            out_word[3] = b64_table[in_word[2] & 0x3F];

            if (n < 3) {
                out_word[3] = '=';
                if (n < 2) {
                    out_word[2] = '=';
                }
            }

            for (n = 0; n < 4; n++) {
                if (out_idx < iOutBufSize && out_word[n] > 0) {
                    *(strOut + out_idx) = out_word[n];
                    out_idx++;
                }
            }

            in_word[0] = in_word[1] = in_word[2] = 0;
            n = 0;
        }
    }
    return out_idx;
}

/* base64_decode
 *   function to decode a string according to base 64 algorithm
 *
 *   strIn : input string to be decoded
 *   strOut : buffer for decoded string
 *   iOutBufSize : size for strOut buffer
 *
 *   return : actual length of output string
 */
INT4
base64_decode
(
    INT1* strIn,
    INT1* strOut,
    INT4 iOutBufSize
)
{
    INT4 iInLen = 0;
    INT4 in_idx = 0;
    INT4 out_idx = 0;
    INT1 in_word[4];
    INT1 out_word[3];
    INT4 i = 0;
    INT4 n = 0;

    if (strIn == NULL || strOut == NULL) {
        return -1;
    }
    if (strlen(strIn) == 0) {
        return -2;
    }
    iInLen = strlen(strIn);
    in_word[0] = in_word[1] = in_word[2] = in_word[3] = 0;
    n = 0;
    for (in_idx = 0; in_idx < iInLen; in_idx++) {
        in_word[n] = *(strIn + in_idx);
        n++;

        if (n == 4 || in_idx == iInLen - 1) {
            for (i = 0; i < 4; i++) {
                if ('A' <= in_word[i] && in_word[i] <= 'Z') {
                    in_word[i] = in_word[i] - 'A';
                }
                else if ('a' <= in_word[i] && in_word[i] <= 'z') {
                    in_word[i] = in_word[i] - 'a' + 26;
                }
                else if ('0' <= in_word[i] && in_word[i] <= '9') {
                    in_word[i] = in_word[i] - '0' + 52;
                }
                else if (in_word[i] == '+') {
                    in_word[i] = 62;
                }
                else if (in_word[i] == '/') {
                    in_word[i] = 63;
                }
                else if (in_word[i] == '=') {
                    in_word[i] = 0;
                }
            }

            out_word[0] = (in_word[0] << 2) | (in_word[1] >> 4);
            out_word[1] = (in_word[1] << 4) | (in_word[2] >> 2);
            out_word[2] = (in_word[2] << 6) | in_word[3];

            for (i = 0; i < 3; i++) {
                if (out_idx < iOutBufSize && out_word[i] > 0) {
                    *(strOut + out_idx) = out_word[i];
                    out_idx++;
                }
            }

            in_word[0] = in_word[1] = in_word[2] = in_word[3] = 0;
            n = 0;
        }
    }
    return out_idx;
}

